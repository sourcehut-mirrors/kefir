/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kefir/driver/parser.h"
#include "kefir/platform/filesystem.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static kefir_bool_t detect_shared_object_file_type(const struct kefir_driver_external_resources *externals,
                                                   const char *filename) {
    const char *full_extension = NULL;
    const char *filename_end = filename + strlen(filename);
    for (const char *c = filename_end - 1; c >= filename; c--) {
        if (*c == '.') {
            full_extension = c;
        } else if (*c == KEFIR_FILESYSTEM_PATH_SEPARATOR) {
            break;
        }
    }

    if (full_extension == NULL) {
        return false;
    }

    kefir_bool_t found = false;
    for (const char **iter = externals->extensions.shared_library; !found && *iter != NULL; ++iter) {
        kefir_size_t suffix_len = strlen(*iter);

        found = true;
        for (const char *full_ext = full_extension; found && *full_ext != '\0';) {
            if (*full_ext != '.' && !isdigit(*full_ext)) {
                found = false;
            } else if (suffix_len > 0 && strncmp(full_ext, *iter, suffix_len) == 0) {
                full_ext += suffix_len;
                suffix_len = 0;
            } else {
                full_ext++;
            }
        }
    }

    return found;
}

static kefir_bool_t check_suffixes(const char *extension, const char **suffixes) {
    for (const char **iter = suffixes; *iter != NULL; ++iter) {
        if (strcmp(extension, *iter) == 0) {
            return true;
        }
    }
    return false;
}

static kefir_driver_argument_type_t detect_file_type(const struct kefir_driver_external_resources *externals,
                                                     const char *filename) {
    const char *extension = NULL;
    for (const char *c = filename + strlen(filename); c > filename; c--) {
        if (*c == '.') {
            extension = c;
            break;
        }
    }
    if (extension == NULL) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE;
    }

    if (check_suffixes(extension, externals->extensions.source_file)) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE;
    } else if (check_suffixes(extension, externals->extensions.preprocessed_file)) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED;
    } else if (check_suffixes(extension, externals->extensions.assembly_file)) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY;
    } else if (check_suffixes(extension, externals->extensions.preprocessed_assembly_file)) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY_WITH_PREPROCESSING;
    } else if (check_suffixes(extension, externals->extensions.object_file) ||
               check_suffixes(extension, externals->extensions.library_object_file)) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT;
    } else if (check_suffixes(extension, externals->extensions.static_library) ||
               detect_shared_object_file_type(externals, filename)) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_LIBRARY;
    } else {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT;
    }
}

#define STRNCMP(_cmp1, _cmp2) (strncmp((_cmp1), (_cmp2), sizeof(_cmp1) - 1))

kefir_result_t kefir_driver_parse_args(struct kefir_mem *mem, struct kefir_string_pool *symbols,
                                       struct kefir_driver_configuration *config,
                                       const struct kefir_driver_external_resources *externals, const char *const *argv,
                                       kefir_size_t argc, kefir_driver_command_t *command, FILE *warning_output) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(symbols != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid symbol table"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(externals != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver external resources"));
    REQUIRE(command != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to driver command"));

    kefir_bool_t override_file_type = false;
    kefir_driver_argument_type_t overriden_arg_type = KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE;
    *command = KEFIR_DRIVER_COMMAND_RUN;
    for (kefir_size_t index = 0; index < argc; index++) {
        const char *arg = argv[index];
#define EXPECT_ARG \
    REQUIRE(index + 1 < argc, KEFIR_SET_ERRORF(KEFIR_UI_ERROR, "Expected an argument for '%s' option", arg))

        // Driver phases
        if (strcmp("-c", arg) == 0) {
            // Suppress linker phase
            config->stage = KEFIR_DRIVER_STAGE_ASSEMBLE;
        } else if (strcmp("-S", arg) == 0) {
            // Suppress assembly phase
            config->stage = KEFIR_DRIVER_STAGE_COMPILE;
        } else if (strcmp("-E", arg) == 0) {
            // Preprocess
            config->stage = KEFIR_DRIVER_STAGE_PREPROCESS;
        } else if (strcmp("-P", arg) == 0) {
            // Preprocess
            config->stage = KEFIR_DRIVER_STAGE_PREPROCESS;
        } else if (strcmp("--preprocess-save", arg) == 0) {
            // Preprocess and save
            config->stage = KEFIR_DRIVER_STAGE_PREPROCESS_SAVE;
        } else if (strcmp("-M", arg) == 0) {
            // Print all dependencies
            config->stage = KEFIR_DRIVER_STAGE_PREPROCESS;
            config->dependency_output.output_dependencies = true;
            config->dependency_output.output_system_deps = true;
        } else if (strcmp("-MM", arg) == 0) {
            // Print all non-system dependencies
            config->stage = KEFIR_DRIVER_STAGE_PREPROCESS;
            config->dependency_output.output_dependencies = true;
            config->dependency_output.output_system_deps = false;
        } else if (strcmp("-MD", arg) == 0) {
            // Print all dependencies
            config->dependency_output.output_dependencies = true;
            config->dependency_output.output_system_deps = true;
        } else if (strcmp("-MMD", arg) == 0) {
            // Print all non-system dependencies
            config->dependency_output.output_dependencies = true;
            config->dependency_output.output_system_deps = false;
        } else if (strcmp("-MP", arg) == 0) {
            // Add phony targets
            config->dependency_output.add_phony_targets = true;
        } else if (strcmp("--print-tokens", arg) == 0) {
            // Print tokens
            config->stage = KEFIR_DRIVER_STAGE_PRINT_TOKENS;
        } else if (strcmp("--print-ast", arg) == 0) {
            // Print AST
            config->stage = KEFIR_DRIVER_STAGE_PRINT_AST;
        } else if (strcmp("--print-ir", arg) == 0) {
            // Print IR
            config->stage = KEFIR_DRIVER_STAGE_PRINT_IR;
        } else if (strcmp("--print-opt", arg) == 0) {
            // Print optimizer code
            config->stage = KEFIR_DRIVER_STAGE_PRINT_OPT;
        } else if (strcmp("-run", arg) == 0) {
            // Run executable
            config->stage = KEFIR_DRIVER_STAGE_RUN;

            if (kefir_list_length(&config->arguments) == 0) {
                EXPECT_ARG;
                arg = argv[++index];
                REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, arg,
                                                                   detect_file_type(externals, arg)));
            }

            for (; index + 1 < argc;) {
                const char *arg = argv[++index];
                arg = kefir_string_pool_insert(mem, symbols, arg, NULL);
                REQUIRE(arg != NULL,
                        KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert run argument into symbols"));

                REQUIRE_OK(
                    kefir_list_insert_after(mem, &config->run.args, kefir_list_tail(&config->run.args), (void *) arg));
            }
        }

        // Generic flags
        else if (STRNCMP("-o", arg) == 0) {
            // Set output file name
            const char *output_filename = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                output_filename = argv[++index];
            } else {
                output_filename = &arg[2];
            }

            config->output_file = kefir_string_pool_insert(mem, symbols, output_filename, NULL);
            REQUIRE(config->output_file != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert output file name into symbols"));
        } else if (strcmp("--target", arg) == 0) {
            // Target specification
            EXPECT_ARG;
            const char *target = argv[++index];

            kefir_result_t res = kefir_driver_target_match(target, &config->target);
            if (res == KEFIR_NOT_FOUND) {
                res = KEFIR_SET_ERRORF(KEFIR_UI_ERROR, "Unknown target '%s'", target);
            }
            REQUIRE_OK(res);
        } else if (strcmp("--restrictive-c", arg) == 0) {
            // Enable restrictive compiler mode
            config->flags.restrictive_mode = true;
        } else if (strcmp("--permissive-c", arg) == 0) {
            // Enable permissive compiler mode
            config->flags.restrictive_mode = false;
        }

        // Compiler flags
        else if (STRNCMP("-std=", arg) == 0) {
            const char *standard_string = arg + 5;
            if (strcmp(standard_string, "c23") == 0 || strcmp(standard_string, "c2x") == 0 ||
                strcmp(standard_string, "iso9899:2024") == 0 || strcmp(standard_string, "gnu23") == 0 ||
                strcmp(standard_string, "gnu2x") == 0) {
                config->standard_version = KEFIR_C23_STANDARD_VERSION;
            } else {
                config->standard_version = KEFIR_C17_STANDARD_VERSION;
            }
        } else if (STRNCMP("-O", arg) == 0) {
            kefir_uint_t level;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                if (isdigit(argv[index + 1][0])) {
                    level = strtoul(argv[++index], NULL, 10);
                } else {
                    level = 1;
                }
            } else {
                if (isdigit(arg[2])) {
                    level = strtoul(&arg[2], NULL, 10);
                } else {
                    level = 1;
                }
            }

            config->compiler.optimization_level = (kefir_int_t) level;
        } else if (strcmp("-fPIC", arg) == 0 || strcmp("-fpic", arg) == 0) {
            config->flags.position_independent_code = true;
        } else if (strcmp("-fno-pic", arg) == 0) {
            config->flags.position_independent_code = false;
        } else if (strcmp("-fno-omit-frame-pointer", arg) == 0) {
            config->flags.omit_frame_pointer = KEFIR_DRIVER_FRAME_POINTER_OMISSION_DISABLE;
        } else if (strcmp("-fomit-frame-pointer", arg) == 0) {
            config->flags.omit_frame_pointer = KEFIR_DRIVER_FRAME_POINTER_OMISSION_ENABLE;
        } else if (strcmp("-g", arg) == 0 || strcmp("-ggdb", arg) == 0) {
            config->flags.debug_info = true;
        } else if (STRNCMP("-ggdb", arg) == 0) {
            kefir_uint_t level = strtoul(&arg[5], NULL, 10);

            config->flags.debug_info = level > 0;
        } else if (STRNCMP("-g", arg) == 0) {
            kefir_uint_t level = strtoul(&arg[2], NULL, 10);

            config->flags.debug_info = level > 0;
        } else if (STRNCMP("-funsigned-char", arg) == 0) {
            config->compiler.char_signedness = KEFIR_DRIVER_CHAR_UNSIGNED;
        } else if (STRNCMP("-fsigned-char", arg) == 0) {
            config->compiler.char_signedness = KEFIR_DRIVER_CHAR_SIGNED;
        } else if (STRNCMP("-fcommon", arg) == 0) {
            config->compiler.tentative_definition_placement = KEFIR_DRIVER_TENTATIVE_DEFINITION_PLACEMENT_COMMON;
        } else if (STRNCMP("-fno-common", arg) == 0) {
            config->compiler.tentative_definition_placement = KEFIR_DRIVER_TENTATIVE_DEFINITION_PLACEMENT_NO_COMMON;
        } else if (STRNCMP("-fvisibility=", arg) == 0) {
            const char *visibility = &arg[13];
            if (strcmp(visibility, "default") == 0) {
                config->compiler.symbol_visibility = KEFIR_DRIVER_SYMBOL_VISIBILITY_DEFAULT;
            } else if (strcmp(visibility, "hidden") == 0) {
                config->compiler.symbol_visibility = KEFIR_DRIVER_SYMBOL_VISIBILITY_HIDDEN;
            } else if (strcmp(visibility, "internal") == 0) {
                config->compiler.symbol_visibility = KEFIR_DRIVER_SYMBOL_VISIBILITY_INTERNAL;
            } else if (strcmp(visibility, "protected") == 0) {
                config->compiler.symbol_visibility = KEFIR_DRIVER_SYMBOL_VISIBILITY_PROTECTED;
            } else if (warning_output != NULL) {
                fprintf(warning_output, "Warning: Unknown visibility setting '%s'\n", visibility);
            }
        } else if (STRNCMP("-x", arg) == 0) {
            const char *language = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                language = argv[++index];
            } else {
                language = &arg[2];
            }

            if (strcmp(language, "none") == 0) {
                override_file_type = false;
            } else if (strcmp(language, "c") == 0 || strcmp(language, "c-header") == 0) {
                override_file_type = true;
                overriden_arg_type = KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE;
            } else if (strcmp(language, "cpp-output") == 0) {
                override_file_type = true;
                overriden_arg_type = KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED;
            } else if (strcmp(language, "assembler") == 0) {
                override_file_type = true;
                overriden_arg_type = KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY;
            } else if (strcmp(language, "assembler-with-cpp") == 0) {
                override_file_type = true;
                overriden_arg_type = KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY_WITH_PREPROCESSING;
            } else if (warning_output != NULL) {
                fprintf(warning_output, "Warning: Unsupported language '%s'\n", language);
            }
        }

        // Preprocessor flags
        else if (strcmp("-fpreprocessed", arg) == 0) {
            config->flags.skip_preprocessor = true;
        } else if (strcmp("--preprocessor-linemarkers", arg) == 0) {
            // Enable preprocessor linemarkers
            config->flags.preprocessor_linemarkers = true;
        } else if (STRNCMP("-D", arg) == 0) {
            // Define macro
            const char *definition = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                definition = argv[++index];
            } else {
                definition = &arg[2];
            }

            const char *delim = strchr(definition, '=');
            const char *name = definition;
            const char *value = NULL;
            if (delim != NULL) {
                value = delim + 1;
                char name_buf[1024] = {0};
                strncpy(name_buf, name, MIN(sizeof(name_buf) - 1, (kefir_size_t) (delim - name)));
                name = kefir_string_pool_insert(mem, symbols, name_buf, NULL);
                REQUIRE(name != NULL,
                        KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert macro definition name into symbols"));
            }
            REQUIRE(strlen(name) > 0, KEFIR_SET_ERROR(KEFIR_UI_ERROR, "Macro name cannot be empty"));

            REQUIRE_OK(kefir_driver_configuration_add_define(mem, symbols, config, name, value));
        } else if (STRNCMP("-U", arg) == 0) {
            // Define macro
            const char *name = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                name = argv[++index];
            } else {
                name = &arg[2];
            }
            REQUIRE_OK(kefir_driver_configuration_add_undefine(mem, symbols, config, name));
        } else if (STRNCMP("-I", arg) == 0) {
            // Add directory to include search path
            const char *directory = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                directory = argv[++index];
            } else {
                directory = &arg[2];
            }

            REQUIRE_OK(kefir_driver_configuration_add_include_directory(mem, symbols, config, directory));
        } else if (STRNCMP("-iquote", arg) == 0) {
            // Add directory to include search path
            const char *directory = NULL;
            if (strlen(arg) == 7) {
                EXPECT_ARG;
                directory = argv[++index];
            } else {
                directory = &arg[7];
            }

            REQUIRE_OK(kefir_driver_configuration_add_quote_include_directory(mem, symbols, config, directory));
        } else if (STRNCMP("-isystem", arg) == 0) {
            // Add directory to include search path
            const char *directory = NULL;
            if (strlen(arg) == 8) {
                EXPECT_ARG;
                directory = argv[++index];
            } else {
                directory = &arg[8];
            }

            REQUIRE_OK(kefir_driver_configuration_add_system_include_directory(mem, symbols, config, directory));
        } else if (STRNCMP("-idirafter", arg) == 0) {
            // Add directory to include search path
            const char *directory = NULL;
            if (strlen(arg) == 10) {
                EXPECT_ARG;
                directory = argv[++index];
            } else {
                directory = &arg[10];
            }

            REQUIRE_OK(kefir_driver_configuration_add_after_include_directory(mem, symbols, config, directory));
        } else if (STRNCMP("--embed-dir=", arg) == 0) {
            // Add directory to embed search path
            const char *directory = &arg[12];
            REQUIRE_OK(kefir_driver_configuration_add_embed_directory(mem, symbols, config, directory));
        } else if (STRNCMP("--embed-dir", arg) == 0) {
            // Add directory to embed search path
            EXPECT_ARG;
            const char *directory = argv[++index];
            REQUIRE_OK(kefir_driver_configuration_add_embed_directory(mem, symbols, config, directory));
        } else if (strcmp("-include", arg) == 0) {
            // Include file
            EXPECT_ARG;
            const char *file = argv[++index];

            REQUIRE_OK(kefir_driver_configuration_add_include_file(mem, symbols, config, file));
        } else if (strcmp("-MT", arg) == 0) {
            // Dependency target name
            EXPECT_ARG;
            const char *target_name = argv[++index];
            target_name = kefir_string_pool_insert(mem, symbols, target_name, NULL);
            REQUIRE(target_name != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert dependency target name into symbols"));

            config->dependency_output.target_name = target_name;
        } else if (strcmp("-MF", arg) == 0) {
            // Dependency output filename
            EXPECT_ARG;
            const char *output_filename;
            if (strlen(arg) > 3) {
                output_filename = &arg[3];
            } else {
                EXPECT_ARG;
                output_filename = argv[++index];
            }
            output_filename = kefir_string_pool_insert(mem, symbols, output_filename, NULL);
            REQUIRE(output_filename != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert dependency target name into symbols"));

            config->dependency_output.output_filename = output_filename;
        }

        // Assembler options
        else if (STRNCMP("-masm", arg) == 0) {
            // Assembler
            const char *assembler = NULL;
            if (strlen(arg) == 5 || arg[5] != '=') {
                EXPECT_ARG;
                assembler = argv[++index];
            } else {
                assembler = &arg[6];
            }

            if (strcmp(assembler, "x86_64-gas-att") == 0 || strcmp(assembler, "att") == 0) {
                config->assembler.target = KEFIR_DRIVER_ASSEMBLER_GAS_ATT;
            } else if (strcmp(assembler, "x86_64-gas-intel") == 0 || strcmp(assembler, "intel") == 0) {
                config->assembler.target = KEFIR_DRIVER_ASSEMBLER_GAS_INTEL;
            } else if (strcmp(assembler, "x86_64-gas-intel_prefix") == 0) {
                config->assembler.target = KEFIR_DRIVER_ASSEMBLER_GAS_INTEL_PREFIX;
            } else if (strcmp(assembler, "x86_64-yasm") == 0) {
                config->assembler.target = KEFIR_DRIVER_ASSEMBLER_YASM;
            } else {
                return KEFIR_SET_ERRORF(KEFIR_UI_ERROR, "Unknown assembler \"%s\"", assembler);
            }
        }

        // Linker flags
        else if (strcmp("-s", arg) == 0) {
            // Strip linked executable
            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, "",
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_STRIP));
        } else if (strcmp("-r", arg) == 0) {
            // Retain relocations
            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, "",
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_RETAIN_RELOC));
        } else if (STRNCMP("-e", arg) == 0) {
            // Set entry point
            const char *entry_point = NULL;
            if (strlen(arg) > 2) {
                entry_point = &arg[2];
            } else {
                EXPECT_ARG;
                entry_point = argv[++index];
            }
            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, entry_point,
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_ENTRY_POINT));
        } else if (STRNCMP("-u", arg) == 0) {
            // Undefined symbol
            const char *symbol = NULL;
            if (strlen(arg) > 2) {
                symbol = &arg[2];
            } else {
                EXPECT_ARG;
                symbol = argv[++index];
            }
            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, symbol,
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_UNDEFINED_SYMBOL));
        } else if (STRNCMP("-l", arg) == 0) {
            // Link library
            const char *library = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                library = argv[++index];
            } else {
                library = &arg[2];
            }

            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, library,
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_LIBRARY));
        } else if (STRNCMP("-L", arg) == 0) {
            // Library search path
            const char *directory = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                directory = argv[++index];
            } else {
                directory = &arg[2];
            }

            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, directory,
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_LINK_PATH));
        } else if (STRNCMP("-rpath", arg) == 0) {
            // Library search path
            const char *directory = NULL;
            if (strlen(arg) == 6 || arg[6] != '=') {
                EXPECT_ARG;
                directory = argv[++index];
            } else {
                directory = &arg[7];
            }

            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, "-rpath",
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA));
            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, directory,
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA));
        } else if (STRNCMP("-soname", arg) == 0) {
            // Shared object name
            const char *soname = NULL;
            if (strlen(arg) == 7 || arg[7] != '=') {
                EXPECT_ARG;
                soname = argv[++index];
            } else {
                soname = &arg[7];
            }

            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, "-soname",
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA));
            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, soname,
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA));
        } else if (strcmp("-rdynamic", arg) == 0) {
            // Export dynamic
            config->flags.export_dynamic = true;
        } else if (strcmp("-static", arg) == 0) {
            // Static linking
            config->flags.static_linking = true;
            config->flags.shared_linking = false;
        } else if (strcmp("-shared", arg) == 0) {
            // Static linking
            config->flags.static_linking = false;
            config->flags.shared_linking = true;
        } else if (strcmp("-pie", arg) == 0) {
            // Position-independent executable
            config->flags.position_independent_executable = true;
        } else if (strcmp("-no-pie", arg) == 0) {
            // Position-independent executable
            config->flags.position_independent_executable = false;
        } else if (strcmp("-nostartfiles", arg) == 0) {
            // Do not link start files
            config->flags.link_start_files = false;
        } else if (strcmp("-nodefaultlibs", arg) == 0) {
            // Do not link default libraries
            config->flags.link_default_libs = false;
        } else if (strcmp("-nolibc", arg) == 0) {
            // Do not link libc
            config->flags.link_libc = false;
        } else if (strcmp("-nostdinc", arg) == 0) {
            // Do not use standard include path
            config->flags.include_stdinc = false;
        } else if (strcmp("-nostdlib", arg) == 0) {
            // Do not link start files and default libraries
            config->flags.link_start_files = false;
            config->flags.link_default_libs = false;
        } else if (strcmp("-nortinc", arg) == 0) {
            // Do not include runtime headers
            config->flags.include_rtinc = false;
        } else if (strcmp("--enable-atomics", arg) == 0) {
            // Enable atomics support
            config->flags.enable_atomics = true;
        } else if (strcmp("--disable-atomics", arg) == 0) {
            // Disable atomics support
            config->flags.enable_atomics = false;
        } else if (strcmp("-pthread", arg) == 0) {
            // Enable pthreads
            config->flags.pthread = true;
        }

        // Run options
        else if (strcmp("-runarg", arg) == 0) {
            // Run argument

            EXPECT_ARG;
            const char *arg = argv[++index];
            arg = kefir_string_pool_insert(mem, symbols, arg, NULL);
            REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert run argument into symbols"));

            REQUIRE_OK(
                kefir_list_insert_after(mem, &config->run.args, kefir_list_tail(&config->run.args), (void *) arg));
        } else if (strcmp("-run-stdin", arg) == 0) {
            // Run stdin
            EXPECT_ARG;
            const char *arg = argv[++index];
            arg = kefir_string_pool_insert(mem, symbols, arg, NULL);
            REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert run argument into symbols"));

            config->run.file_stdin = arg;
        } else if (strcmp("-run-stdout", arg) == 0) {
            // Run stdout
            EXPECT_ARG;
            const char *arg = argv[++index];
            arg = kefir_string_pool_insert(mem, symbols, arg, NULL);
            REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert run argument into symbols"));

            config->run.file_stdout = arg;
        } else if (strcmp("-run-stderr", arg) == 0) {
            // Run stderr
            EXPECT_ARG;
            const char *arg = argv[++index];
            arg = kefir_string_pool_insert(mem, symbols, arg, NULL);
            REQUIRE(arg != NULL, KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert run argument into symbols"));

            config->run.stderr_to_stdout = false;
            config->run.file_stderr = arg;
        } else if (strcmp("-run-stderr2out", arg) == 0) {
            // Run stderr to stdout
            config->run.stderr_to_stdout = true;
            config->run.file_stderr = NULL;
        }

        // Extra tool options
#define SPLIT_OPTIONS(_init, _callback)                                                                              \
    do {                                                                                                             \
        const char *options = (_init);                                                                               \
        for (const char *next_comma = strchr(options, ','); next_comma != NULL; next_comma = strchr(options, ',')) { \
            const kefir_size_t option_length = next_comma - options;                                                 \
            char *copy = KEFIR_MALLOC(mem, sizeof(char) * (option_length + 1));                                      \
            REQUIRE(copy != NULL,                                                                                    \
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate external tool option copy"));        \
            strncpy(copy, options, option_length);                                                                   \
            copy[option_length] = '\0';                                                                              \
            kefir_result_t res = _callback(mem, symbols, config, copy);                                              \
            KEFIR_FREE(mem, copy);                                                                                   \
            REQUIRE_OK(res);                                                                                         \
            options = next_comma + 1;                                                                                \
        }                                                                                                            \
        if (*options != '\0') {                                                                                      \
            REQUIRE_OK(_callback(mem, symbols, config, options));                                                    \
        }                                                                                                            \
    } while (0)
#define ADD_LINKER_ARG(_mem, _symbols, _config, _option)                              \
    kefir_driver_configuration_add_argument((_mem), (_symbols), (_config), (_option), \
                                            KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA)

        else if (STRNCMP("-Wa,", arg) == 0) {
            // Assembler options
            SPLIT_OPTIONS(arg + 4, kefir_driver_configuration_add_assembler_argument);
        } else if (strcmp("-Xassembler", arg) == 0) {
            // Assembler options
            EXPECT_ARG;
            const char *flag = argv[++index];
            REQUIRE_OK(kefir_driver_configuration_add_assembler_argument(mem, symbols, config, flag));
        } else if (STRNCMP("-Wl,", arg) == 0) {
            // Linker options
            SPLIT_OPTIONS(arg + 4, ADD_LINKER_ARG);
        } else if (STRNCMP("-z", arg) == 0) {
            // Linker options
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                arg = argv[++index];
            } else {
                arg = &arg[2];
            }
            REQUIRE_OK(ADD_LINKER_ARG(mem, symbols, config, "-z"));
            REQUIRE_OK(ADD_LINKER_ARG(mem, symbols, config, arg));
        } else if (strcmp("-Xlinker", arg) == 0) {
            // Linker options
            EXPECT_ARG;
            const char *flag = argv[++index];
            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, flag,
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA));
        } else if (STRNCMP("-Wp,", arg) == 0 || STRNCMP("-Wc,", arg) == 0) {
            // Preprocessor and compiler options
            SPLIT_OPTIONS(arg + 4, kefir_driver_configuration_add_compiler_argument);
        } else if (strcmp("-Xpreprocessor", arg) == 0) {
            // Preprocessor: ignored
            EXPECT_ARG;
            const char *flag = argv[++index];
            REQUIRE_OK(kefir_driver_configuration_add_compiler_argument(mem, symbols, config, flag));
        } else if (STRNCMP("-W", arg) == 0) {
            // Tool options
            kefir_bool_t next_arg = false;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                arg = argv[++index];
                next_arg = true;
            } else {
                arg = &arg[2];
            }
            if (STRNCMP("a,", arg) == 0) {
                // Assembler options
                SPLIT_OPTIONS(arg + 2, kefir_driver_configuration_add_assembler_argument);
            } else if (STRNCMP("l,", arg) == 0) {
                // Linker options
                SPLIT_OPTIONS(arg + 2, ADD_LINKER_ARG);
            } else if (STRNCMP("p,", arg) == 0 || STRNCMP("c,", arg) == 0) {
                // Compiler and linker options
                SPLIT_OPTIONS(arg + 2, kefir_driver_configuration_add_compiler_argument);
            } else {
                // Other options
                char buffer[512];
                if (!next_arg) {
                    snprintf(buffer, sizeof(buffer) - 1, "--%s", arg);
                    arg = buffer;
                }
                REQUIRE_OK(kefir_driver_configuration_add_compiler_argument(mem, symbols, config, arg));
            }
        }
#undef ADD_LINKER_ARG
#undef SPLIT_OPTIONS

        // Other flags
        else if (strcmp("-h", arg) == 0 || strcmp("--help", arg) == 0) {
            // Help requested
            *command = KEFIR_DRIVER_COMMAND_HELP;
            return KEFIR_OK;
        } else if (strcmp("-v", arg) == 0 || strcmp("--version", arg) == 0) {
            // Version requested
            *command = KEFIR_DRIVER_COMMAND_VERSION;
            return KEFIR_OK;
        } else if (strcmp("--compiler-info", arg) == 0) {
            // Compiler info requested
            *command = KEFIR_DRIVER_COMMAND_COMPILER_INFO;
            return KEFIR_OK;
        } else if (strcmp("--environment-info", arg) == 0) {
            // Environment info requested
            *command = KEFIR_DRIVER_COMMAND_COMPILER_ENVIRONMENT;
            return KEFIR_OK;
        } else if (strcmp("--environment-header", arg) == 0) {
            // Environment info requested
            *command = KEFIR_DRIVER_COMMAND_TARGET_ENVIRONMENT_HEADER;
            return KEFIR_OK;
        } else if (strcmp("-verbose", arg) == 0) {
            // Verbose driver output
            config->flags.verbose = true;
        }

        // Ignored unsupported flags
        else if (strcmp("-", arg) == 0) {
            // Positional argument
            REQUIRE_OK(
                kefir_driver_configuration_add_argument(mem, symbols, config, arg, !override_file_type ? detect_file_type(externals, arg) : overriden_arg_type));
        } else if (STRNCMP("-", arg) == 0 || STRNCMP("--", arg) == 0) {
            // All other non-positional arguments: ignored
            if (warning_output != NULL) {
                fprintf(warning_output, "Warning: Unsupported command line option '%s'\n", arg);
            }
        }

        // Positional argument
        else {
            REQUIRE_OK(
                kefir_driver_configuration_add_argument(mem, symbols, config, arg, !override_file_type ? detect_file_type(externals, arg) : overriden_arg_type));
        }

#undef EXPECT_ARG
    }
    return KEFIR_OK;
}
#undef STRNCMP

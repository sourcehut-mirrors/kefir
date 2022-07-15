/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>
#include <stdio.h>

static kefir_driver_argument_type_t detect_file_type(const char *filename) {
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

    if (strcmp(extension, ".c") == 0) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE;
    } else if (strcmp(extension, ".i") == 0) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_PREPROCESSED;
    } else if (strcmp(extension, ".s") == 0) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_ASSEMBLY;
    } else if (strcmp(extension, ".o") == 0) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_OBJECT;
    } else if (strcmp(extension, ".a") == 0) {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_LIBRARY;
    } else {
        return KEFIR_DRIVER_ARGUMENT_INPUT_FILE_CODE;
    }
}

kefir_result_t kefir_driver_parse_args(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                       struct kefir_driver_configuration *config, const char *const *argv,
                                       kefir_size_t argc, kefir_driver_command_t *command, FILE *warning_output) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(symbols != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid symbol table"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(command != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to driver command"));

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
            // Preprocess and save
            config->stage = KEFIR_DRIVER_STAGE_PREPROCESS_SAVE;
        } else if (strcmp("--print-tokens", arg) == 0) {
            // Print tokens
            config->stage = KEFIR_DRIVER_STAGE_PRINT_TOKENS;
        } else if (strcmp("--print-ast", arg) == 0) {
            // Print AST
            config->stage = KEFIR_DRIVER_STAGE_PRINT_AST;
        } else if (strcmp("--print-ir", arg) == 0) {
            // Print IR
            config->stage = KEFIR_DRIVER_STAGE_PRINT_IR;
        } else if (strcmp("--print-runtime-code", arg) == 0) {
            // Print IR
            config->stage = KEFIR_DRIVER_STAGE_PRINT_RUNTIME_CODE;
        }

        // Generic flags
        else if (strncmp("-o", arg, 2) == 0) {
            // Set output file name
            const char *output_filename = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                output_filename = argv[++index];
            } else {
                output_filename = &arg[2];
            }

            config->output_file = kefir_symbol_table_insert(mem, symbols, output_filename, NULL);
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

        // Preprocessor flags
        else if (strncmp("-D", arg, 2) == 0) {
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
                name = kefir_symbol_table_insert(mem, symbols, name_buf, NULL);
                REQUIRE(name != NULL,
                        KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert macro definition name into symbols"));
            }
            REQUIRE(strlen(name) > 0, KEFIR_SET_ERROR(KEFIR_UI_ERROR, "Macro name cannot be empty"));

            REQUIRE_OK(kefir_driver_configuration_add_define(mem, symbols, config, name, value));
        } else if (strncmp("-U", arg, 2) == 0) {
            // Define macro
            const char *name = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                name = argv[++index];
            } else {
                name = &arg[2];
            }
            REQUIRE_OK(kefir_driver_configuration_add_undefine(mem, symbols, config, name));
        } else if (strncmp("-I", arg, 2) == 0) {
            // Add directory to include search path
            const char *directory = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                directory = argv[++index];
            } else {
                directory = &arg[2];
            }

            REQUIRE_OK(kefir_driver_configuration_add_include_directory(mem, symbols, config, directory));
        } else if (strcmp("-include", arg) == 0) {
            // Include file
            EXPECT_ARG;
            const char *file = argv[++index];

            REQUIRE_OK(kefir_driver_configuration_add_include_file(mem, symbols, config, file));
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
        } else if (strncmp("-e", arg, 2) == 0) {
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
        } else if (strncmp("-u", arg, 2) == 0) {
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
        } else if (strncmp("-l", arg, 2) == 0) {
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
        } else if (strncmp("-L", arg, 2) == 0) {
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
        } else if (strcmp("-static", arg) == 0) {
            // Static linking
            config->flags.static_linking = true;
        } else if (strcmp("-nostartfiles", arg) == 0) {
            // Do not link start files
            config->flags.link_start_files = false;
        } else if (strcmp("-nodefaultlibs", arg) == 0) {
            // Do not link default libraries
            config->flags.link_default_libs = false;
        } else if (strcmp("-nolibc", arg) == 0) {
            // Do not link libc
            config->flags.link_libc = false;
        } else if (strcmp("-nostdlib", arg) == 0) {
            // Do not link start files and default libraries
            config->flags.link_start_files = false;
            config->flags.link_default_libs = false;
        } else if (strcmp("-nortlib", arg) == 0) {
            // Do not link runtime library
            config->flags.link_rtlib = false;
        }

        // Extra tool options
        else if (strncmp("-Wa,", arg, 4) == 0) {
            // Assembler options
            REQUIRE_OK(kefir_driver_configuration_add_assembler_argument(mem, symbols, config, arg + 4));
        } else if (strcmp("-Xassembler", arg) == 0) {
            // Assembler options
            EXPECT_ARG;
            const char *flag = argv[++index];
            REQUIRE_OK(kefir_driver_configuration_add_assembler_argument(mem, symbols, config, flag));
        } else if (strncmp("-Wl,", arg, 4) == 0) {
            // Linker options
            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, arg + 4,
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_ENTRY_POINT));
        } else if (strcmp("-Xlinker", arg) == 0) {
            // Linker options
            EXPECT_ARG;
            const char *flag = argv[++index];
            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, flag,
                                                               KEFIR_DRIVER_ARGUMENT_LINKER_FLAG_EXTRA));
        } else if (strncmp("-Wp,", arg, 4) == 0 || strncmp("-Wc,", arg, 4) == 0) {
            // Preprocessor and compiler options
            REQUIRE_OK(kefir_driver_configuration_add_compiler_argument(mem, symbols, config, arg + 4));
        } else if (strcmp("-Xpreprocessor", arg) == 0) {
            // Preprocessor: ignored
            EXPECT_ARG;
            const char *flag = argv[++index];
            REQUIRE_OK(kefir_driver_configuration_add_compiler_argument(mem, symbols, config, flag));
        } else if (strncmp("-W", arg, 2) == 0) {
            // Tool options
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                arg = argv[++index];
            } else {
                arg = &arg[2];
            }
            if (strncmp("a,", arg, 2) == 0) {
                // Assembler options
                REQUIRE_OK(kefir_driver_configuration_add_assembler_argument(mem, symbols, config, arg + 2));
            } else if (strncmp("l,", arg, 2) == 0) {
                // Linker options
                REQUIRE_OK(kefir_driver_configuration_add_assembler_argument(mem, symbols, config, arg + 2));
            } else if (strncmp("p,", arg, 2) == 0 || strncmp("c,", arg, 2) == 0) {
                // Compiler and linker options
                REQUIRE_OK(kefir_driver_configuration_add_compiler_argument(mem, symbols, config, arg + 2));
            } else {
                // Other options
                REQUIRE_OK(kefir_driver_configuration_add_compiler_argument(mem, symbols, config, arg));
            }
        }

        // Other flags
        else if (strcmp("-h", arg) == 0 || strcmp("--help", arg) == 0) {
            // Help requested
            *command = KEFIR_DRIVER_COMMAND_HELP;
            return KEFIR_OK;
        } else if (strcmp("-v", arg) == 0 || strcmp("--version", arg) == 0) {
            // Version requested
            *command = KEFIR_DRIVER_COMMAND_VERSION;
            return KEFIR_OK;
        }

        // Ignored unsupported flags
        else if (strncmp("-O", arg, 2) == 0) {
            // Optimization level: ignored
            if (warning_output != NULL) {
                fprintf(warning_output, "Warning: Unsupported command line option '%s'\n", arg);
            }
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                ++index;
            }
        } else if (strncmp("-x", arg, 2) == 0) {
            // Language: ignored
            if (warning_output != NULL) {
                fprintf(warning_output, "Warning: Unsupported command line option '%s'\n", arg);
            }
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                ++index;
            }
        } else if (strcmp("-", arg) == 0) {
            // Positional argument
            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, arg, detect_file_type(arg)));
        } else if (strncmp("-", arg, 1) == 0 || strncmp("--", arg, 2) == 0) {
            // All other non-positional arguments: ignored
            if (warning_output != NULL) {
                fprintf(warning_output, "Warning: Unsupported command line option '%s'\n", arg);
            }
        }

        // Positional argument
        else {
            REQUIRE_OK(kefir_driver_configuration_add_argument(mem, symbols, config, arg, detect_file_type(arg)));
        }

#undef EXPECT_ARG
    }
    return KEFIR_OK;
}

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

static kefir_driver_input_file_type_t detect_file_type(const char *filename) {
    const char *extension = NULL;
    for (const char *c = filename + strlen(filename); c > filename; c--) {
        if (*c == '.') {
            extension = c;
            break;
        }
    }
    if (extension == NULL) {
        return KEFIR_DRIVER_INPUT_FILE_CODE;
    }

    if (strcmp(extension, ".c") == 0) {
        return KEFIR_DRIVER_INPUT_FILE_CODE;
    } else if (strcmp(extension, ".i") == 0) {
        return KEFIR_DRIVER_INPUT_FILE_PREPROCESSED;
    } else if (strcmp(extension, ".s") == 0) {
        return KEFIR_DRIVER_INPUT_FILE_ASSEMBLY;
    } else if (strcmp(extension, ".o") == 0) {
        return KEFIR_DRIVER_INPUT_FILE_OBJECT;
    } else if (strcmp(extension, ".a") == 0) {
        return KEFIR_DRIVER_INPUT_FILE_LIBRARY;
    } else {
        return KEFIR_DRIVER_INPUT_FILE_CODE;
    }
}

kefir_result_t kefir_driver_parse_args(struct kefir_mem *mem, struct kefir_symbol_table *symbols,
                                       struct kefir_driver_configuration *config, const char *const *argv,
                                       kefir_size_t argc, kefir_bool_t *help_requested) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(symbols != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid symbol table"));
    REQUIRE(config != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid driver configuration"));
    REQUIRE(help_requested != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to help requested flag"));

    for (kefir_size_t index = 0; index < argc; index++) {
        const char *arg = argv[index];
#define EXPECT_ARG \
    REQUIRE(index + 1 < argc, KEFIR_SET_ERRORF(KEFIR_UI_ERROR, "Expected an argument for '%s' option", arg))

        if (strcmp("-c", arg) == 0) {
            // Suppress linker phase
            config->stage = KEFIR_DRIVER_STAGE_ASSEMBLE;
        } else if (strcmp("-S", arg) == 0) {
            // Suppress assembly phase
            config->stage = KEFIR_DRIVER_STAGE_COMPILE;
        } else if (strcmp("-s", arg) == 0) {
            // Strip linked executable
            REQUIRE_OK(
                kefir_driver_configuration_add_linker_flag(mem, symbols, config, "", KEFIR_DRIVER_LINKER_FLAG_STRIP));
        } else if (strcmp("-r", arg) == 0) {
            // Retain relocations
            REQUIRE_OK(kefir_driver_configuration_add_linker_flag(mem, symbols, config, "",
                                                                  KEFIR_DRIVER_LINKER_FLAG_RETAIN_RELOC));
        } else if (strncmp("-e", arg, 2) == 0) {
            // Set entry point
            const char *entry_point = NULL;
            if (strlen(arg) > 2) {
                entry_point = &arg[2];
            } else {
                EXPECT_ARG;
                entry_point = argv[++index];
            }
            REQUIRE_OK(kefir_driver_configuration_add_linker_flag(mem, symbols, config, entry_point,
                                                                  KEFIR_DRIVER_LINKER_FLAG_ENTRY_POINT));
        } else if (strncmp("-u", arg, 2) == 0) {
            // Undefined symbol
            const char *symbol = NULL;
            if (strlen(arg) > 2) {
                symbol = &arg[2];
            } else {
                EXPECT_ARG;
                symbol = argv[++index];
            }
            REQUIRE_OK(kefir_driver_configuration_add_linker_flag(mem, symbols, config, symbol,
                                                                  KEFIR_DRIVER_LINKER_FLAG_UNDEFINED_SYMBOL));
        } else if (strcmp("-o", arg) == 0) {
            // Set output file name
            EXPECT_ARG;
            config->output_file = kefir_symbol_table_insert(mem, symbols, argv[++index], NULL);
            REQUIRE(config->output_file != NULL,
                    KEFIR_SET_ERROR(KEFIR_OBJALLOC_FAILURE, "Failed to insert output file name into symbols"));
        } else if (strncmp("-D", arg, 2) == 0) {
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
                char name_buf[1024];
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
        } else if (strcmp("-E", arg) == 0) {
            // Preprocess
            config->stage = KEFIR_DRIVER_STAGE_PREPROCESS;
        } else if (strcmp("-P", arg) == 0) {
            // Preprocess and save
            config->stage = KEFIR_DRIVER_STAGE_PREPROCESS_SAVE;
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
        } else if (strncmp("-l", arg, 2) == 0) {
            // Link library
            const char *library = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                library = argv[++index];
            } else {
                library = &arg[2];
            }

            REQUIRE_OK(kefir_driver_configuration_add_linker_flag(mem, symbols, config, library,
                                                                  KEFIR_DRIVER_LINKER_FLAG_LINK_LIBRARY));
        } else if (strncmp("-L", arg, 2) == 0) {
            // Library search path
            const char *directory = NULL;
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                directory = argv[++index];
            } else {
                directory = &arg[2];
            }

            REQUIRE_OK(kefir_driver_configuration_add_linker_flag(mem, symbols, config, directory,
                                                                  KEFIR_DRIVER_LINKER_FLAG_LINK_PATH));
        } else if (strncmp("-Wa,", arg, 4) == 0) {
            // Assembler options
            REQUIRE_OK(kefir_driver_configuration_add_assembler_extra_flag(mem, symbols, config, arg + 4));
        } else if (strncmp("-Wl,", arg, 4) == 0) {
            // Linker options
            REQUIRE_OK(kefir_driver_configuration_add_linker_flag(mem, symbols, config, arg + 4,
                                                                  KEFIR_DRIVER_LINKER_FLAG_EXTRA));
        } else if (strncmp("-Wp", arg, 3) == 0 || strncmp("-Wc", arg, 3) == 0) {
            // Preprocessor and compiler options: ignored
        } else if (strcmp("-W", arg) == 0) {
            // Tool options
            EXPECT_ARG;
            arg = argv[++index];
            if (strncmp("a,", arg, 2) == 0) {
                // Assembler options
                REQUIRE_OK(kefir_driver_configuration_add_assembler_extra_flag(mem, symbols, config, arg + 2));
            } else if (strncmp("l,", arg, 2) == 0) {
                // Linker options
                REQUIRE_OK(kefir_driver_configuration_add_assembler_extra_flag(mem, symbols, config, arg + 2));
            } else {
                // Other options: ignored
            }
        } else if (strcmp("-g", arg) == 0) {
            // Produce debug info: ignored
        } else if (strcmp("-f", arg) == 0) {
            // Enable floating-point support: ignored
        } else if (strncmp("-O", arg, 2) == 0) {
            // Optimization level: ignored
            if (strlen(arg) == 2) {
                EXPECT_ARG;
                ++index;
            }
        } else if (strcmp("-p", arg) == 0 || strcmp("-q", arg) == 0) {
            // Profiling: ignored
        } else if (strcmp("-C", arg) == 0) {
            // Preserve comments after preprocessing: ignored
        } else if (strcmp("-h", arg) == 0 || strcmp("--help", arg) == 0) {
            // Help requested
            *help_requested = true;
            return KEFIR_OK;
        } else if (strncmp("-", arg, 1) == 0 || strncmp("--", arg, 2) == 0) {
            // All other non-positional arguments: ignored
        } else {
            // Positional argument
            REQUIRE_OK(kefir_driver_configuration_add_input(mem, symbols, config, arg, detect_file_type(arg)));
        }
    }
    return KEFIR_OK;
}

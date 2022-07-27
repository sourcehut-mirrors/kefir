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

#include "kefir/standalone/options.h"
#include "kefir/driver/compiler_options.h"
#include "kefir/platform/cli_parser.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"
#include <string.h>

kefir_result_t kefir_cli_parse_runner_configuration(struct kefir_mem *mem,
                                                    struct kefir_compiler_runner_configuration *options,
                                                    char *const *argv, kefir_size_t argc,
                                                    kefir_cli_command_t *cmd_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(options != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to cli options"));
    REQUIRE(argv != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid argument list"));
    REQUIRE(cmd_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to CLI command"));

    if (argc > 1 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        *cmd_ptr = KEFIR_CLI_COMMAND_HELP;
        return KEFIR_OK;
    } else if (argc > 1 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
        *cmd_ptr = KEFIR_CLI_COMMAND_VERSION;
        return KEFIR_OK;
    } else {
        *cmd_ptr = KEFIR_CLI_COMMAND_RUN;
    }

    kefir_size_t positional_args = argc;
    REQUIRE_OK(kefir_parse_cli_options(mem, NULL, options, &positional_args, KefirCompilerConfigurationOptions,
                                       KefirCompilerConfigurationOptionCount, argv, argc));
    if (positional_args < argc) {
        REQUIRE(positional_args + 1 == argc,
                KEFIR_SET_ERROR(KEFIR_UI_ERROR, "Cannot specify more than one input file"));
        options->input_filepath = argv[positional_args];
    }
    return KEFIR_OK;
}

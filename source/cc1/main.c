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

#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include "kefir/compiler/compiler.h"
#include "kefir/driver/runner.h"
#include "kefir/core/version.h"
#include "kefir/cc1/options.h"

// Standalone compiler without driver

extern const char KefirHelpContent[];

int main(int argc, char *const *argv) {
    struct kefir_mem *mem = kefir_system_memalloc();
    struct kefir_compiler_runner_configuration options;
    kefir_cli_command_t cli_cmd;

    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");
    kefir_result_t res = kefir_compiler_runner_configuration_init(&options);
    REQUIRE_CHAIN(&res, kefir_cli_parse_runner_configuration(mem, &options, argv, argc, &cli_cmd));
    if (res == KEFIR_OK) {
        switch (cli_cmd) {
            case KEFIR_CLI_COMMAND_RUN:
                REQUIRE_CHAIN(&res, kefir_run_compiler(mem, &options));
                REQUIRE_CHAIN(&res, kefir_compiler_runner_configuration_free(mem, &options));
                break;

            case KEFIR_CLI_COMMAND_HELP:
                fprintf(stdout, "%s", KefirHelpContent);
                break;

            case KEFIR_CLI_COMMAND_VERSION:
                fprintf(stdout, "%u.%u.%u\n", KEFIR_VERSION_MAJOR, KEFIR_VERSION_MINOR, KEFIR_VERSION_PATCH);
                break;
        }
    }
    return kefir_report_error(stderr, res, options.error_report_type == KEFIR_COMPILER_RUNNER_ERROR_REPORT_JSON)
               ? EXIT_SUCCESS
               : EXIT_FAILURE;
}

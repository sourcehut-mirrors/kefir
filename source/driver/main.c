/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include <signal.h>

// Driver main entry

#define KEFIR_DRIVER_PROLOGUE_INTERNAL
#include "kefir/driver/driver_prologue.h"

int main(int argc, char *const *argv) {
    UNUSED(argc);
    init_tmpmgr();
    kefir_result_t res = KEFIR_OK;
    struct kefir_mem *mem = kefir_system_memalloc();

    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC, "C");

    struct kefir_string_pool symbols;
    struct kefir_driver_configuration driver_config;
    struct kefir_driver_external_resources exteral_resources;
    kefir_driver_command_t command;
    int exit_code = EXIT_SUCCESS;

    REQUIRE_CHAIN(&res, kefir_string_pool_init(&symbols));
    REQUIRE_CHAIN(&res, kefir_driver_configuration_init(&driver_config));
    REQUIRE_CHAIN(&res, kefir_driver_external_resources_init_from_env(mem, &exteral_resources, &tmpmgr));
    if (exteral_resources.default_target != NULL) {
        REQUIRE_CHAIN(&res, kefir_driver_target_match(exteral_resources.default_target, &driver_config.target));
    }
    REQUIRE_CHAIN(&res, kefir_driver_parse_args(mem, &symbols, &driver_config, &exteral_resources,
                                                (const char *const *) argv + 1, argc - 1, &command, stderr));
    if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_HELP) {
        fprintf(stdout, "%s", KefirDriverHelpContent);
    } else if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_VERSION) {
        fprintf(stdout, "%s\n", KEFIR_VERSION_FULL);
    } else if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_COMPILER_INFO) {
        res = print_compiler_info(stdout, argv[0]);
    } else if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_COMPILER_ENVIRONMENT) {
        res = print_environment(stdout, &driver_config.target, &exteral_resources);
    } else if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_TARGET_ENVIRONMENT_HEADER) {
        res = print_target_environment_header(stdout, &driver_config.target, &exteral_resources);
    } else {
        REQUIRE_CHAIN(&res, kefir_driver_run(mem, &symbols, &driver_config, &exteral_resources));
        if (res == KEFIR_INTERRUPT) {
            res = KEFIR_OK;
            exit_code = EXIT_FAILURE;
        }
    }

    REQUIRE_CHAIN(&res, kefir_driver_configuration_free(mem, &driver_config));
    REQUIRE_CHAIN(&res, kefir_string_pool_free(mem, &symbols));
    return kefir_report_error(stderr, res, false) ? exit_code : EXIT_FAILURE;
}

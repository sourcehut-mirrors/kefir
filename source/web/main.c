/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#define KEFIR_DRIVER_PROLOGUE_INTERNAL
#include "kefir/driver/driver_prologue.h"

int kefir_run_with_args(int argc, char *const *argv) {
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
    REQUIRE_CHAIN(&res, kefir_driver_parse_args(mem, &symbols, &driver_config, (const char *const *) argv + 1, argc - 1,
                                                &command, stderr));
    if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_HELP) {
        fprintf(stdout, "%s", KefirDriverHelpContent);
    } else if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_VERSION) {
        fprintf(stdout, "%u.%u.%u\n", KEFIR_VERSION_MAJOR, KEFIR_VERSION_MINOR, KEFIR_VERSION_PATCH);
    } else if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_COMPILER_INFO) {
        res = print_compiler_info(stdout, argv[0]);
    } else if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_COMPILER_ENVIRONMENT) {
        res = print_environment(stdout, &exteral_resources);
    } else if (res == KEFIR_OK && command == KEFIR_DRIVER_COMMAND_TARGET_ENVIRONMENT_HEADER) {
        res = print_target_environment_header(stdout, &driver_config.target, &exteral_resources);
    } else {
        struct kefir_compiler_runner_configuration compiler_config;
        REQUIRE_CHAIN(&res, kefir_driver_generate_compiler_config(mem, &symbols, &driver_config, &exteral_resources,
                                                                  &compiler_config));
        compiler_config.input_filepath = NULL;
        compiler_config.output_filepath = NULL;
        REQUIRE_CHAIN(&res, kefir_run_compiler(mem, &compiler_config));
        REQUIRE_CHAIN(&res, kefir_compiler_runner_configuration_free(mem, &compiler_config));
        if (res == KEFIR_INTERRUPT) {
            res = KEFIR_OK;
            exit_code = EXIT_FAILURE;
        }
    }

    REQUIRE_CHAIN(&res, kefir_driver_configuration_free(mem, &driver_config));
    REQUIRE_CHAIN(&res, kefir_string_pool_free(mem, &symbols));
    return kefir_report_error(stderr, res, false) ? exit_code : EXIT_FAILURE;
}

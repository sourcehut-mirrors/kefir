/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2021  Jevgenijs Protopopovs

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

#define _POSIX_SOURCE
#include "kefir/main/driver.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/os_error.h"
#include "kefir/compiler/compiler.h"
#include "kefir/main/runner.h"
#include <unistd.h>
#include <sys/wait.h>

static int run_compiler(void *payload) {
    ASSIGN_DECL_CAST(const struct kefir_compiler_runner_configuration *, configuration, payload);
    kefir_result_t res = kefir_run_compiler(kefir_system_memalloc(), configuration);
    return kefir_report_error(stderr, res, configuration) ? EXIT_SUCCESS : EXIT_FAILURE;
}

kefir_result_t kefir_driver_run_compiler(const struct kefir_compiler_runner_configuration *configuration,
                                         struct kefir_process *process) {
    REQUIRE(configuration != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler runner configuration"));
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));

    REQUIRE_OK(kefir_process_run(process, run_compiler, (void *) configuration));
    return KEFIR_OK;
}

kefir_result_t kefir_driver_run_assembler(const char *output_file, struct kefir_process *process) {
    REQUIRE(output_file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid output file"));
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));

    const char *as_path = getenv("KEFIR_AS");
    if (as_path == NULL) {
        as_path = "as";
    }

    const char *argv[] = {as_path, "-o", output_file, NULL};

    REQUIRE_OK(kefir_process_execute(process, as_path, argv));
    return KEFIR_OK;
}

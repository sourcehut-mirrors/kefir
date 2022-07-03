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

kefir_result_t kefir_driver_run_compiler(const struct kefir_compiler_runner_configuration *configuration,
                                         FILE *input_fp, FILE *output_fp, FILE *error_fp) {
    REQUIRE(configuration != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid compiler runner configuration"));
    REQUIRE(input_fp != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid stdin file"));
    REQUIRE(output_fp != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid stdout file"));
    REQUIRE(error_fp != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid stderr file"));

    int pid = fork();
    REQUIRE(pid >= 0, KEFIR_SET_OS_ERROR("Failed to fork compiler runner"));
    if (pid > 0) {
        // Driver process
        int runner_status;
        REQUIRE(waitpid(pid, &runner_status, 0) != -1, KEFIR_SET_OS_ERROR("Failed to wait for runner process"));
        if (WIFEXITED(runner_status)) {
            REQUIRE(WEXITSTATUS(runner_status) == EXIT_SUCCESS, KEFIR_INTERRUPT);
        } else if (WIFSIGNALED(runner_status)) {
            return KEFIR_SET_ERRORF(KEFIR_SUBPROCESS_ERROR, "Runner process failed due to signal %d",
                                    WTERMSIG(runner_status));
        } else {
            return KEFIR_SET_ERROR(KEFIR_SUBPROCESS_ERROR, "Runner process failed due to unknown reasons");
        }
    } else {
        // Runner process
        struct kefir_mem *mem = kefir_system_memalloc();

        kefir_result_t res = KEFIR_OK;
        REQUIRE_CHAIN_SET(&res, dup2(fileno(input_fp), STDIN_FILENO) != -1,
                          KEFIR_SET_OS_ERROR("Failed to set up runner process stdin"));
        REQUIRE_CHAIN_SET(&res, dup2(fileno(output_fp), STDIN_FILENO) != -1,
                          KEFIR_SET_OS_ERROR("Failed to set up runner process stdout"));
        REQUIRE_CHAIN_SET(&res, dup2(fileno(error_fp), STDIN_FILENO) != -1,
                          KEFIR_SET_OS_ERROR("Failed to set up runner process stderr"));
        REQUIRE_CHAIN(&res, kefir_run_compiler(mem, configuration));
        exit(kefir_report_error(stderr, res, configuration) ? EXIT_SUCCESS : EXIT_FAILURE);
    }
    return KEFIR_OK;
}

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
#include "kefir/util/process.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/os_error.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

kefir_result_t kefir_process_init(struct kefir_process *process) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));

    process->pid = -1;
    process->io.stdin = dup(STDIN_FILENO);
    REQUIRE(process->io.stdin != -1, KEFIR_SET_OS_ERROR("Failed to duplicate process stdin"));
    process->io.stdout = dup(STDOUT_FILENO);
    REQUIRE_ELSE(process->io.stdout != -1, {
        kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to duplicate process stdout");
        close(process->io.stdin);
        return res;
    });
    process->io.stderr = dup(STDERR_FILENO);
    REQUIRE_ELSE(process->io.stderr != -1, {
        kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to duplicate process stderr");
        close(process->io.stdin);
        close(process->io.stdout);
        return res;
    });
    process->status.exited = false;
    process->status.terminated = false;
    return KEFIR_OK;
}

kefir_result_t kefir_process_wait(struct kefir_process *process) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    REQUIRE(process->pid > 0, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected running process"));

    int status;
    REQUIRE(waitpid(process->pid, &status, 0) != -1, KEFIR_SET_OS_ERROR("Failed to wait for process"));
    if (WIFEXITED(status)) {
        process->status.exited = true;
        process->status.exit_code = WEXITSTATUS(status);
        process->pid = -1;
    } else if (WIFSIGNALED(status)) {
        process->status.terminated = true;
        process->status.signal = WTERMSIG(status);
        process->pid = -1;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_process_free(struct kefir_process *process) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    if (process->pid > 0) {
        REQUIRE(kill(process->pid, SIGKILL) != -1, KEFIR_SET_OS_ERROR("Failed to kill running process"));
        process->pid = -1;
    }

    REQUIRE(close(process->io.stdin) != -1, KEFIR_SET_OS_ERROR("Failed to close process stdin"));
    process->io.stdin = -1;
    REQUIRE(close(process->io.stdout) != -1, KEFIR_SET_OS_ERROR("Failed to close process stdout"));
    process->io.stdout = -1;
    REQUIRE(close(process->io.stderr) != -1, KEFIR_SET_OS_ERROR("Failed to close process stderr"));
    process->io.stderr = -1;
    return KEFIR_OK;
}

kefir_result_t kefir_process_run(struct kefir_process *process, int (*callback)(void *), void *payload) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    REQUIRE(process->pid == -1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected initialized process"));

    int pid = fork();
    REQUIRE(pid != -1, KEFIR_SET_OS_ERROR("Failed to fork the process"));
    if (pid > 0) {
        process->pid = pid;
    } else {
        if (dup2(process->io.stdin, STDIN_FILENO) == -1) {
            perror("Failed to set up process stdin");
            exit(EXIT_FAILURE);
        }
        if (dup2(process->io.stdout, STDOUT_FILENO) == -1) {
            perror("Failed to set up process stdout");
            exit(EXIT_FAILURE);
        }
        if (dup2(process->io.stderr, STDERR_FILENO) == -1) {
            perror("Failed to set up process stderr");
            exit(EXIT_FAILURE);
        }

        exit(callback(payload));
    }
    return KEFIR_OK;
}

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

#define _POSIX_SOURCE
#include "kefir/util/process.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/os_error.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

static kefir_bool_t process_is_fork = false;

kefir_result_t kefir_process_init(struct kefir_process *process) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));

    process->pid = -1;
    process->io.input_fd = dup(STDIN_FILENO);
    REQUIRE(process->io.input_fd != -1, KEFIR_SET_OS_ERROR("Failed to duplicate process stdin"));
    process->io.output_fd = dup(STDOUT_FILENO);
    REQUIRE_ELSE(process->io.output_fd != -1, {
        kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to duplicate process stdout");
        close(process->io.input_fd);
        return res;
    });
    process->io.error_fd = dup(STDERR_FILENO);
    REQUIRE_ELSE(process->io.error_fd != -1, {
        kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to duplicate process stderr");
        close(process->io.input_fd);
        close(process->io.output_fd);
        return res;
    });
    process->status.exited = false;
    process->status.terminated = false;
    return KEFIR_OK;
}

static kefir_result_t close_process(struct kefir_process *process) {
    process->pid = -1;
    REQUIRE(close(process->io.input_fd) != -1, KEFIR_SET_OS_ERROR("Failed to close process stdin"));
    process->io.input_fd = -1;
    REQUIRE(close(process->io.output_fd) != -1, KEFIR_SET_OS_ERROR("Failed to close process stdout"));
    process->io.output_fd = -1;
    REQUIRE(close(process->io.error_fd) != -1, KEFIR_SET_OS_ERROR("Failed to close process stderr"));
    process->io.error_fd = -1;
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
        REQUIRE_OK(close_process(process));
    } else if (WIFSIGNALED(status)) {
        process->status.terminated = true;
        process->status.signal = WTERMSIG(status);
        REQUIRE_OK(close_process(process));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_process_kill(struct kefir_process *process) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    if (process->pid > 0) {
        REQUIRE(kill(process->pid, SIGKILL) != -1, KEFIR_SET_OS_ERROR("Failed to kill running process"));
        REQUIRE_OK(close_process(process));
        process->status.terminated = true;
        process->status.signal = SIGKILL;
    }
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
        process_is_fork = true;
        if (dup2(process->io.input_fd, STDIN_FILENO) == -1) {
            perror("Failed to set up process stdin");
            exit(EXIT_FAILURE);
        }
        if (dup2(process->io.output_fd, STDOUT_FILENO) == -1) {
            perror("Failed to set up process stdout");
            exit(EXIT_FAILURE);
        }
        if (dup2(process->io.error_fd, STDERR_FILENO) == -1) {
            perror("Failed to set up process stderr");
            exit(EXIT_FAILURE);
        }

        exit(callback(payload));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_process_execute(struct kefir_process *process, const char *file, char *const *args) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    REQUIRE(process->pid == -1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected initialized process"));
    REQUIRE(file != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid executable file"));
    REQUIRE(args != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid executable arguments"));

    int pid = fork();
    REQUIRE(pid != -1, KEFIR_SET_OS_ERROR("Failed to fork the process"));
    if (pid > 0) {
        process->pid = pid;
    } else {
        process_is_fork = true;
        if (dup2(process->io.input_fd, STDIN_FILENO) == -1) {
            perror("Failed to set up process stdin");
            exit(EXIT_FAILURE);
        }
        if (dup2(process->io.output_fd, STDOUT_FILENO) == -1) {
            perror("Failed to set up process stdout");
            exit(EXIT_FAILURE);
        }
        if (dup2(process->io.error_fd, STDERR_FILENO) == -1) {
            perror("Failed to set up process stderr");
            exit(EXIT_FAILURE);
        }

        if (execvp(file, (char *const *) args) == -1) {
            perror("Failed to execute file");
            exit(EXIT_FAILURE);
        }
        abort();  // Execution flow shall not reach this statement
    }
    return KEFIR_OK;
}

kefir_result_t kefir_process_pipe(struct kefir_process *src_process, struct kefir_process *dst_process) {
    REQUIRE(src_process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to source process"));
    REQUIRE(src_process->pid == -1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected initialized source process"));
    REQUIRE(dst_process != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to destination process"));
    REQUIRE(dst_process->pid == -1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected initialized destination process"));

    int pipe_fd[2];
    REQUIRE(pipe(pipe_fd) != -1, KEFIR_SET_OS_ERROR("Failed to create pipe"));

    int fd_flags;
    if (fcntl(pipe_fd[0], F_GETFD, &fd_flags) == -1 || fcntl(pipe_fd[0], F_SETFD, fd_flags | FD_CLOEXEC) == -1 ||
        fcntl(pipe_fd[1], F_GETFD, &fd_flags) == -1 || fcntl(pipe_fd[1], F_SETFD, fd_flags | FD_CLOEXEC) == -1) {
        kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to close process stdin");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return res;
    }

    REQUIRE_ELSE(close(dst_process->io.input_fd) == 0, {
        kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to close process stdin");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return res;
    });
    dst_process->io.input_fd = pipe_fd[0];

    REQUIRE_ELSE(close(src_process->io.output_fd) == 0, {
        kefir_result_t res = KEFIR_SET_OS_ERROR("Failed to close process stdout");
        close(pipe_fd[1]);
        return res;
    });
    src_process->io.output_fd = pipe_fd[1];
    return KEFIR_OK;
}

kefir_result_t kefir_process_redirect_stderr_to_stdout(struct kefir_process *process) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    REQUIRE(process->pid == -1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected initialized process"));

    REQUIRE(close(process->io.error_fd) == 0, KEFIR_SET_OS_ERROR("Failed to close process stderr"));
    process->io.error_fd = dup(process->io.output_fd);
    REQUIRE(process->io.error_fd != -1, KEFIR_SET_OS_ERROR("Failed to redirect stderr"));
    return KEFIR_OK;
}

kefir_result_t kefir_process_redirect_stdin_from(struct kefir_process *process, int fd) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    REQUIRE(process->pid == -1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected initialized process"));
    REQUIRE(fd >= 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid file descriptor"));

    REQUIRE(close(process->io.input_fd) == 0, KEFIR_SET_OS_ERROR("Failed to close process stdin"));
    process->io.input_fd = fd;
    return KEFIR_OK;
}

kefir_result_t kefir_process_redirect_stdout_to(struct kefir_process *process, int fd) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    REQUIRE(process->pid == -1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected initialized process"));
    REQUIRE(fd >= 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid file descriptor"));

    REQUIRE(close(process->io.output_fd) == 0, KEFIR_SET_OS_ERROR("Failed to close process stdout"));
    process->io.output_fd = fd;
    return KEFIR_OK;
}

kefir_result_t kefir_process_redirect_stderr_to(struct kefir_process *process, int fd) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    REQUIRE(process->pid == -1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected initialized process"));
    REQUIRE(fd >= 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid file descriptor"));

    REQUIRE(close(process->io.error_fd) == 0, KEFIR_SET_OS_ERROR("Failed to close process stderr"));
    process->io.error_fd = fd;
    return KEFIR_OK;
}

kefir_result_t kefir_process_redirect_stdin_from_file(struct kefir_process *process, const char *filepath) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    REQUIRE(process->pid == -1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected initialized process"));
    REQUIRE(filepath, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid file path"));

    int fd = open(filepath, O_RDONLY);
    REQUIRE(fd >= 0, KEFIR_SET_OS_ERROR("Failed to attach file to process stdin"));
    kefir_result_t res = kefir_process_redirect_stdin_from(process, fd);
    REQUIRE_ELSE(res == KEFIR_OK, {
        close(fd);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_process_redirect_stdout_to_file(struct kefir_process *process, const char *filepath) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    REQUIRE(process->pid == -1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected initialized process"));
    REQUIRE(filepath, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid file path"));

    int fd = open(filepath, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);
    REQUIRE(fd >= 0, KEFIR_SET_OS_ERROR("Failed to redirect process stdout to file"));
    kefir_result_t res = kefir_process_redirect_stdout_to(process, fd);
    REQUIRE_ELSE(res == KEFIR_OK, {
        close(fd);
        return res;
    });
    return KEFIR_OK;
}

kefir_result_t kefir_process_redirect_stderr_to_file(struct kefir_process *process, const char *filepath) {
    REQUIRE(process != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to process"));
    REQUIRE(process->pid == -1, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Expected initialized process"));
    REQUIRE(filepath, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid file path"));

    int fd = open(filepath, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR);
    REQUIRE(fd >= 0, KEFIR_SET_OS_ERROR("Failed to redirect process stderr to file"));
    kefir_result_t res = kefir_process_redirect_stderr_to(process, fd);
    REQUIRE_ELSE(res == KEFIR_OK, {
        close(fd);
        return res;
    });
    return KEFIR_OK;
}

kefir_bool_t kefir_process_is_fork() {
    return process_is_fork;
}

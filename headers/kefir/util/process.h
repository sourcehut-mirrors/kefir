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

#ifndef KEFIR_UTIL_PROCESS_H_
#define KEFIR_UTIL_PROCESS_H_

#include "kefir/core/basic-types.h"

typedef struct kefir_process {
    int pid;
    struct {
        int input_fd;
        int output_fd;
        int error_fd;
    } io;

    struct {
        kefir_bool_t exited;
        kefir_bool_t terminated;
        union {
            int exit_code;
            int signal;
        };
    } status;
} kefir_process_t;

kefir_result_t kefir_process_init(struct kefir_process *);
kefir_result_t kefir_process_wait(struct kefir_process *);
kefir_result_t kefir_process_free(struct kefir_process *);

kefir_result_t kefir_process_run(struct kefir_process *, int (*)(void *), void *);
kefir_result_t kefir_process_execute(struct kefir_process *, const char *, const char *const *);

kefir_result_t kefir_process_pipe(struct kefir_process *, struct kefir_process *, kefir_bool_t);

#endif

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

#ifndef KEFIR_MAIN_DRIVER_H_
#define KEFIR_MAIN_DRIVER_H_

#include "kefir/cli/options.h"
#include "kefir/util/process.h"
#include "kefir/core/list.h"
#include <stdio.h>

typedef struct kefir_driver_assembler_configuration {
    struct kefir_list extra_args;
} kefir_driver_assembler_configuration_t;

kefir_result_t kefir_driver_assembler_configuration_init(struct kefir_driver_assembler_configuration *);
kefir_result_t kefir_driver_assembler_configuration_free(struct kefir_mem *,
                                                         struct kefir_driver_assembler_configuration *);
kefir_result_t kefir_driver_assembler_configuration_add_extra_argument(struct kefir_mem *,
                                                                       struct kefir_driver_assembler_configuration *,
                                                                       const char *);

typedef struct kefir_driver_linker_configuration {
    struct kefir_list linked_files;
    struct kefir_list extra_args;
} kefir_driver_linker_configuration_t;

kefir_result_t kefir_driver_linker_configuration_init(struct kefir_driver_linker_configuration *);
kefir_result_t kefir_driver_linker_configuration_free(struct kefir_mem *, struct kefir_driver_linker_configuration *);
kefir_result_t kefir_driver_linker_configuration_add_linked_file(struct kefir_mem *,
                                                                 struct kefir_driver_linker_configuration *,
                                                                 const char *);
kefir_result_t kefir_driver_linker_configuration_add_extra_argument(struct kefir_mem *,
                                                                    struct kefir_driver_linker_configuration *,
                                                                    const char *);

kefir_result_t kefir_driver_run_compiler(const struct kefir_compiler_runner_configuration *, struct kefir_process *);
kefir_result_t kefir_driver_run_assembler(struct kefir_mem *, const char *,
                                          const struct kefir_driver_assembler_configuration *, struct kefir_process *);
kefir_result_t kefir_driver_run_linker(struct kefir_mem *, const char *,
                                       const struct kefir_driver_linker_configuration *, struct kefir_process *);

#endif

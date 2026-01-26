/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#ifndef KEFIR_OPTIMIZER_TOPOLOGICAL_SCHEDULE_H_
#define KEFIR_OPTIMIZER_TOPOLOGICAL_SCHEDULE_H_

#include "kefir/optimizer/schedule.h"

typedef kefir_result_t (*kefir_opt_code_topological_scheduler_instruction_dependency_callback_t)(kefir_opt_instruction_ref_t, void *);
typedef kefir_result_t (*kefir_opt_code_topological_scheduler_instruction_callback_t)(kefir_opt_instruction_ref_t, kefir_opt_code_topological_scheduler_instruction_dependency_callback_t, void *, kefir_bool_t *, void *);

typedef struct kefir_opt_code_topological_scheduler {
    struct kefir_opt_code_scheduler scheduler;
    kefir_opt_code_topological_scheduler_instruction_callback_t instr_callback;
    void *instr_callback_payload;
} kefir_opt_code_topological_scheduler_t;

kefir_result_t kefir_opt_code_topological_scheduler_init(struct kefir_opt_code_topological_scheduler *, kefir_opt_code_topological_scheduler_instruction_callback_t, void *);

kefir_result_t kefir_opt_code_topological_scheduler_default_schedule(
    kefir_opt_instruction_ref_t,
    kefir_opt_code_topological_scheduler_instruction_dependency_callback_t, void *,
    kefir_bool_t *, void *);

#endif

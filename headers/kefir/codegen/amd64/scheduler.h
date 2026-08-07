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

#ifndef KEFIR_CODEGEN_AMD64_SCHEDULER_H_
#define KEFIR_CODEGEN_AMD64_SCHEDULER_H_

#include "kefir/optimizer/topological_schedule.h"
#include "kefir/codegen/amd64/function.h"

typedef struct kefir_codegen_amd64_function_schedule_instruction_parameters {
    struct kefir_mem *mem;
    struct kefir_codegen_amd64_function *func;
} kefir_codegen_amd64_function_schedule_instruction_parameters_t;

kefir_result_t kefir_codegen_amd64_function_schedule_instruction(
    kefir_opt_instruction_ref_t, kefir_opt_code_topological_scheduler_instruction_dependency_callback_t, void *, kefir_bool_t *, void *);

#endif

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

#ifndef KEFIR_CODEGEN_FUNCTION_H_
#define KEFIR_CODEGEN_FUNCTION_H_

#include "kefir/optimizer/code.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/optimizer/linear_liveness.h"
#include "kefir/optimizer/liveness.h"
#include "kefir/optimizer/schedule.h"
#include "kefir/optimizer/variable_scope.h"
#include "kefir/codegen/asmcmp/type_defs.h"
#include "kefir/codegen/variable_allocator.h"

typedef struct kefir_codegen_function {
    struct kefir_opt_code_control_flow control_flow;
    struct kefir_opt_code_liveness liveness;
    struct kefir_opt_code_variable_scopes variable_scopes;
    struct kefir_codegen_local_variable_allocator variable_allocator;
    struct kefir_opt_code_schedule schedule;
    struct kefir_opt_code_linear_liveness linear_liveness;
    const struct kefir_opt_module *module;
    struct kefir_opt_function *function;

    kefir_result_t (*resolve_virtual_register)(kefir_opt_instruction_ref_t, kefir_asmcmp_virtual_register_index_t *, void *);
    void *payload;
} kefir_codegen_function_t;

kefir_result_t kefir_codegen_function_init(const struct kefir_opt_module *, struct kefir_opt_function *, struct kefir_codegen_function *);
kefir_result_t kefir_codegen_function_free(struct kefir_mem *, struct kefir_codegen_function *);

#endif

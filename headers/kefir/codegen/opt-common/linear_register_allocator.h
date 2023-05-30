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

#ifndef KEFIR_CODEGEN_OPT_COMMON_LINEAR_REGISTER_ALLOCTOR_H_
#define KEFIR_CODEGEN_OPT_COMMON_LINEAR_REGISTER_ALLOCTOR_H_

#include "kefir/optimizer/analysis.h"
#include "kefir/codegen/opt-common/virtual_register_allocator.h"
#include "kefir/core/graph.h"

typedef enum kefir_codegen_opt_linear_register_allocator_constraint_type {
    KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_SKIP_ALLOCATION,
    KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_GENERAL_PURPOSE,
    KEFIR_CODEGEN_OPT_LINEAR_REGISTER_ALLOCATOR_CONSTRAINT_FLOATING_POINT
} kefir_codegen_opt_linear_register_allocator_constraint_type_t;

typedef struct kefir_codegen_opt_linear_register_allocator_constraint {
    kefir_codegen_opt_linear_register_allocator_constraint_type_t type;
    struct {
        kefir_bool_t present;
        kefir_codegen_opt_virtual_register_t vreg;
    } register_hint;

    struct {
        kefir_bool_t present;
        kefir_opt_instruction_ref_t alias;
    } alias_hint;
} kefir_codegen_opt_linear_register_allocator_constraint_t;

typedef struct kefir_codegen_opt_linear_register_allocation {
    kefir_bool_t done;
    kefir_codegen_opt_virtual_register_t allocation;
    struct kefir_codegen_opt_linear_register_allocator_constraint constraint;
} kefir_codegen_opt_linear_register_allocation_t;

typedef struct kefir_codegen_opt_linear_register_allocator {
    const struct kefir_opt_code_analysis *analysis;
    struct kefir_graph allocation;
    struct kefir_codegen_opt_virtual_register_allocator general_purpose;
    struct kefir_codegen_opt_virtual_register_allocator floating_point;
} kefir_codegen_opt_linear_register_allocator_t;

kefir_result_t kefir_codegen_opt_linear_register_allocator_init(struct kefir_mem *,
                                                                struct kefir_codegen_opt_linear_register_allocator *,
                                                                const struct kefir_opt_code_analysis *, kefir_size_t,
                                                                kefir_size_t);
kefir_result_t kefir_codegen_opt_linear_register_allocator_free(struct kefir_mem *,
                                                                struct kefir_codegen_opt_linear_register_allocator *);

kefir_result_t kefir_codegen_opt_linear_register_allocator_set_type(
    const struct kefir_codegen_opt_linear_register_allocator *, kefir_opt_instruction_ref_t,
    kefir_codegen_opt_linear_register_allocator_constraint_type_t);

kefir_result_t kefir_codegen_opt_linear_register_allocator_hint_register(
    const struct kefir_codegen_opt_linear_register_allocator *, kefir_opt_instruction_ref_t,
    kefir_codegen_opt_virtual_register_t);

kefir_result_t kefir_codegen_opt_linear_register_allocator_hint_alias(
    const struct kefir_codegen_opt_linear_register_allocator *, kefir_opt_instruction_ref_t,
    kefir_opt_instruction_ref_t);

kefir_result_t kefir_codegen_opt_linear_register_allocator_run(struct kefir_mem *,
                                                               struct kefir_codegen_opt_linear_register_allocator *);

kefir_result_t kefir_codegen_opt_linear_register_allocator_allocation_of(
    const struct kefir_codegen_opt_linear_register_allocator *, kefir_opt_instruction_ref_t,
    const struct kefir_codegen_opt_linear_register_allocation **);

#endif

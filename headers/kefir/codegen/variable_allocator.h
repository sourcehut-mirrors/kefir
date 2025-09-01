/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

#ifndef KEFIR_CODEGEN_VARIABLE_ALLOCATOR_H_
#define KEFIR_CODEGEN_VARIABLE_ALLOCATOR_H_

#include "kefir/optimizer/local_variables.h"
#include "kefir/core/hashset.h"

typedef struct kefir_codegen_local_variable_allocator_hooks {
    kefir_result_t (*type_layout)(kefir_id_t, kefir_size_t, kefir_size_t *, kefir_size_t *, void *);
    void *payload;
} kefir_codegen_local_variable_allocator_hooks_t;

typedef struct kefir_codegen_local_variable_allocator {
    struct kefir_hashset alive_variables;
    struct kefir_hashtree variable_locations;
    kefir_opt_instruction_ref_t return_space_variable_ref;
    kefir_size_t total_size;
    kefir_size_t total_alignment;
    kefir_bool_t all_global;
} kefir_codegen_local_variable_allocator_t;

typedef enum kefir_codegen_local_variable_allocation_type {
    KEFIR_CODEGEN_LOCAL_VARIABLE_STACK_ALLOCATION,
    KEFIR_CODEGEN_LOCAL_VARIABLE_RETURN_SPACE
} kefir_codegen_local_variable_allocation_type_t;

kefir_result_t kefir_codegen_local_variable_allocator_init(struct kefir_codegen_local_variable_allocator *);
kefir_result_t kefir_codegen_local_variable_allocator_free(struct kefir_mem *,
                                                           struct kefir_codegen_local_variable_allocator *);

kefir_result_t kefir_codegen_local_variable_allocator_mark_alive(struct kefir_mem *,
                                                                 struct kefir_codegen_local_variable_allocator *,
                                                                 kefir_opt_instruction_ref_t, kefir_id_t *);
kefir_result_t kefir_codegen_local_variable_allocator_mark_all_global(struct kefir_codegen_local_variable_allocator *);
kefir_result_t kefir_codegen_local_variable_allocator_mark_return_space(struct kefir_codegen_local_variable_allocator *,
                                                                        kefir_opt_instruction_ref_t);
kefir_result_t kefir_codegen_local_variable_allocator_run(struct kefir_mem *,
                                                          struct kefir_codegen_local_variable_allocator *,
                                                          const struct kefir_opt_code_container *,
                                                          const struct kefir_codegen_local_variable_allocator_hooks *,
                                                          const struct kefir_opt_code_variable_conflicts *);
kefir_result_t kefir_codegen_local_variable_allocation_of(const struct kefir_codegen_local_variable_allocator *,
                                                          kefir_opt_instruction_ref_t,
                                                          kefir_codegen_local_variable_allocation_type_t *,
                                                          kefir_int64_t *);

#endif

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

#ifndef KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATOR_H_
#define KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATOR_H_

#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/amd64/stack_frame.h"
#include "kefir/core/graph.h"
#include "kefir/core/bitset.h"

typedef enum kefir_codegen_amd64_register_allocation_type {
    KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED,
    KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER,
    KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT,
    KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_INDIRECT,
    KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER
} kefir_codegen_amd64_register_allocation_type_t;

typedef struct kefir_codegen_amd64_register_allocation {
    kefir_asmcmp_virtual_register_index_t vreg;
    kefir_codegen_amd64_register_allocation_type_t type;
    union {
        kefir_asm_amd64_xasmgen_register_t direct_reg;
        struct {
            kefir_size_t index;
            kefir_size_t length;
        } spill_area;
    };
    struct {
        kefir_size_t begin;
        kefir_size_t end;
    } lifetime;
} kefir_codegen_amd64_register_allocation_t;

typedef struct kefir_codegen_amd64_register_allocator {
    struct kefir_codegen_amd64_register_allocation *allocations;
    kefir_size_t num_of_vregs;
    struct kefir_hashtreeset used_registers;
    struct {
        struct kefir_graph liveness_graph;
        struct kefir_hashtree instruction_linear_indices;
        struct kefir_hashtreeset alive_virtual_registers;
        struct kefir_hashtreeset conflicting_requirements;
        struct kefir_bitset spill_area;
        kefir_asm_amd64_xasmgen_register_t *gp_register_allocation_order;
        kefir_size_t num_of_gp_registers;
        kefir_asm_amd64_xasmgen_register_t *sse_register_allocation_order;
        kefir_size_t num_of_sse_registers;
    } internal;
} kefir_codegen_amd64_register_allocator_t;

kefir_result_t kefir_codegen_amd64_register_allocator_init(struct kefir_codegen_amd64_register_allocator *);
kefir_result_t kefir_codegen_amd64_register_allocator_free(struct kefir_mem *,
                                                           struct kefir_codegen_amd64_register_allocator *);

kefir_result_t kefir_codegen_amd64_register_allocator_run(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                          struct kefir_codegen_amd64_stack_frame *,
                                                          struct kefir_codegen_amd64_register_allocator *);

kefir_result_t kefir_codegen_amd64_register_allocation_of(const struct kefir_codegen_amd64_register_allocator *,
                                                          kefir_asmcmp_virtual_register_index_t,
                                                          const struct kefir_codegen_amd64_register_allocation **);

kefir_result_t kefir_codegen_amd64_register_allocator_linear_position_of(
    const struct kefir_codegen_amd64_register_allocator *, kefir_asmcmp_instruction_index_t, kefir_size_t *);

#endif

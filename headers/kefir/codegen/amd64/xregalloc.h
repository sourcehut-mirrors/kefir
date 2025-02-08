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

#ifndef KEFIR_CODEGEN_AMD64_XREGALLOC_H_
#define KEFIR_CODEGEN_AMD64_XREGALLOC_H_

#include "kefir/codegen/amd64/asmcmp.h"
#include "kefir/codegen/amd64/stack_frame.h"
#include "kefir/codegen/asmcmp/liveness.h"
#include "kefir/core/graph.h"
#include "kefir/core/bitset.h"
#include "kefir/core/bucketset.h"

#define KEFIR_CODEGEN_AMD64_XREGALLOC_UNDEFINED ((kefir_size_t) -1ll)
#define KEFIR_CODEGEN_AMD64_XREGALLOC_VIRTUAL_BLOCK_DEFAULT_ID ((kefir_uint64_t) ~0ull)

typedef kefir_uint64_t kefir_codegen_amd64_xregalloc_virtual_block_id_t;

typedef enum kefir_codegen_amd64_register_allocation_type {
    KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED,
    KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER,
    KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT,
    KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_INDIRECT,
    KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER
} kefir_codegen_amd64_register_allocation_type_t;

typedef struct kefir_codegen_amd64_register_allocation {
    kefir_codegen_amd64_register_allocation_type_t type;
    union {
        kefir_asm_amd64_xasmgen_register_t direct_reg;
        struct {
            kefir_size_t index;
            kefir_size_t length;
        } spill_area;
    };
} kefir_codegen_amd64_register_allocation_t;

typedef struct kefir_codegen_amd64_xregalloc_virtual_register {
    struct kefir_codegen_amd64_register_allocation allocation;
    struct {
        kefir_size_t begin;
        kefir_size_t end;
    } lifetime;
    struct kefir_bucketset interference;
    struct kefir_bucketset virtual_blocks;
} kefir_codegen_amd64_xregalloc_virtual_register_t;

typedef struct kefir_codegen_amd64_xregalloc {
    kefir_size_t *linearized_code;
    kefir_size_t linear_code_length;

    struct kefir_codegen_amd64_xregalloc_virtual_register *virtual_registers;
    kefir_size_t virtual_register_length;

    struct {
        kefir_asm_amd64_xasmgen_register_t *general_purpose_registers;
        kefir_size_t num_of_general_purpose_registers;

        kefir_asm_amd64_xasmgen_register_t *floating_point_registers;
        kefir_size_t num_of_floating_point_registers;
    } available_registers;
    struct kefir_hashtreeset used_registers;
    kefir_size_t used_slots;
    struct kefir_hashtree virtual_blocks;
} kefir_codegen_amd64_xregalloc_t;

kefir_result_t kefir_codegen_amd64_xregalloc_init(struct kefir_codegen_amd64_xregalloc *);
kefir_result_t kefir_codegen_amd64_xregalloc_free(struct kefir_mem *, struct kefir_codegen_amd64_xregalloc *);

kefir_result_t kefir_codegen_amd64_xregalloc_run(struct kefir_mem *, struct kefir_asmcmp_amd64 *,
                                                          struct kefir_codegen_amd64_stack_frame *,
                                                          struct kefir_codegen_amd64_xregalloc *);

kefir_result_t kefir_codegen_amd64_xregalloc_allocation_of(const struct kefir_codegen_amd64_xregalloc *,
                                                          kefir_asmcmp_virtual_register_index_t,
                                                          const struct kefir_codegen_amd64_register_allocation **);

kefir_result_t kefir_codegen_amd64_xregalloc_linear_position_of(
    const struct kefir_codegen_amd64_xregalloc *, kefir_asmcmp_instruction_index_t, kefir_size_t *);

kefir_result_t kefir_codegen_amd64_xregalloc_lifetime_of(
    const struct kefir_codegen_amd64_xregalloc *, kefir_asmcmp_virtual_register_index_t, kefir_size_t *, kefir_size_t *);

kefir_result_t kefir_codegen_amd64_xregalloc_exists_in_block(
    const struct kefir_codegen_amd64_xregalloc *, kefir_asmcmp_virtual_register_index_t, kefir_codegen_amd64_xregalloc_virtual_block_id_t, kefir_bool_t *);

kefir_bool_t kefir_codegen_amd64_xregalloc_has_used_register(const struct kefir_codegen_amd64_xregalloc *, kefir_asm_amd64_xasmgen_register_t);

typedef struct kefir_codegen_amd64_xregalloc_virtual_block_iterator {
    struct virtual_block_data *virtual_block;
    struct kefir_bucketset_iterator iter;
} kefir_codegen_amd64_xregalloc_virtual_block_iterator_t;

kefir_result_t kefir_codegen_amd64_xregalloc_block_iter(const struct kefir_codegen_amd64_xregalloc *, kefir_codegen_amd64_xregalloc_virtual_block_id_t, struct kefir_codegen_amd64_xregalloc_virtual_block_iterator *, kefir_asmcmp_virtual_register_index_t *);
kefir_result_t kefir_codegen_amd64_xregalloc_block_next(struct kefir_codegen_amd64_xregalloc_virtual_block_iterator *, kefir_asmcmp_virtual_register_index_t *);

#endif

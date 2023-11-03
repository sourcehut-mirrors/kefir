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

#include "kefir/codegen/amd64/register_allocator.h"
#include "kefir/target/abi/amd64/function.h"
#include "kefir/core/sort.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/list.h"
#include <string.h>

static const kefir_asm_amd64_xasmgen_register_t AMD64_GENERAL_PURPOSE_REGS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_RAX, KEFIR_AMD64_XASMGEN_REGISTER_RBX, KEFIR_AMD64_XASMGEN_REGISTER_RCX,
    KEFIR_AMD64_XASMGEN_REGISTER_RDX, KEFIR_AMD64_XASMGEN_REGISTER_RSI, KEFIR_AMD64_XASMGEN_REGISTER_RDI,
    KEFIR_AMD64_XASMGEN_REGISTER_R8,  KEFIR_AMD64_XASMGEN_REGISTER_R9,  KEFIR_AMD64_XASMGEN_REGISTER_R10,
    KEFIR_AMD64_XASMGEN_REGISTER_R11, KEFIR_AMD64_XASMGEN_REGISTER_R12, KEFIR_AMD64_XASMGEN_REGISTER_R13,
    KEFIR_AMD64_XASMGEN_REGISTER_R14, KEFIR_AMD64_XASMGEN_REGISTER_R15};

const kefir_size_t NUM_OF_AMD64_GENERAL_PURPOSE_REGS =
    sizeof(AMD64_GENERAL_PURPOSE_REGS) / sizeof(AMD64_GENERAL_PURPOSE_REGS[0]);

kefir_result_t kefir_codegen_amd64_register_allocator_init(struct kefir_codegen_amd64_register_allocator *allocator) {
    REQUIRE(allocator != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register allocator"));

    REQUIRE_OK(kefir_graph_init(&allocator->internal.liveness_graph, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&allocator->used_registers, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&allocator->internal.instruction_linear_indices, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&allocator->internal.conflicting_requirements, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&allocator->internal.alive_virtual_registers, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_bitset_init(&allocator->internal.spill_area));
    allocator->allocations = NULL;
    allocator->num_of_vregs = 0;
    allocator->internal.gp_register_allocation_order = NULL;
    allocator->internal.num_of_gp_registers = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_register_allocator_free(struct kefir_mem *mem,
                                                           struct kefir_codegen_amd64_register_allocator *allocator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));

    REQUIRE_OK(kefir_bitset_free(mem, &allocator->internal.spill_area));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &allocator->internal.alive_virtual_registers));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &allocator->internal.conflicting_requirements));
    REQUIRE_OK(kefir_hashtree_free(mem, &allocator->internal.instruction_linear_indices));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &allocator->used_registers));
    REQUIRE_OK(kefir_graph_free(mem, &allocator->internal.liveness_graph));
    if (allocator->allocations != NULL) {
        KEFIR_FREE(mem, allocator->allocations);
    }
    if (allocator->internal.gp_register_allocation_order != NULL) {
        KEFIR_FREE(mem, allocator->internal.gp_register_allocation_order);
    }
    memset(allocator, 0, sizeof(struct kefir_codegen_amd64_register_allocator));
    return KEFIR_OK;
}

static kefir_result_t update_virtual_register_lifetime(struct kefir_asmcmp_amd64 *target,
                                                       struct kefir_codegen_amd64_register_allocator *allocator,
                                                       const struct kefir_asmcmp_value *value,
                                                       kefir_size_t lifetime_index) {
    kefir_asmcmp_virtual_register_index_t vreg;
    struct kefir_codegen_amd64_register_allocation *alloc;

#define UPDATE_LIFETIME(_vreg)                                                  \
    do {                                                                        \
        alloc = &allocator->allocations[(_vreg)];                               \
        if (alloc->lifetime.begin == KEFIR_ASMCMP_INDEX_NONE) {                 \
            alloc->lifetime.begin = lifetime_index;                             \
            alloc->lifetime.end = lifetime_index;                               \
        } else {                                                                \
            alloc->lifetime.begin = MIN(alloc->lifetime.begin, lifetime_index); \
            alloc->lifetime.end = MAX(alloc->lifetime.end, lifetime_index);     \
        }                                                                       \
    } while (false)
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT:
        case KEFIR_ASMCMP_VALUE_TYPE_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            UPDATE_LIFETIME(value->vreg.index);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    UPDATE_LIFETIME(value->indirect.base.vreg);
                    break;

                case KEFIR_ASMCMP_INDIRECT_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_TEMPORARY_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX:
            REQUIRE_OK(kefir_asmcmp_register_stash_vreg(&target->context, value->stash_idx, &vreg));
            UPDATE_LIFETIME(vreg);
            break;
    }
#undef UPDATE_LIFETIME
    return KEFIR_OK;
}

static kefir_result_t calculate_lifetimes(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                          struct kefir_codegen_amd64_register_allocator *allocator) {
    kefir_size_t linear_index = 0;
    for (kefir_asmcmp_instruction_index_t instr_index = kefir_asmcmp_context_instr_head(&target->context);
         instr_index != KEFIR_ASMCMP_INDEX_NONE;
         instr_index = kefir_asmcmp_context_instr_next(&target->context, instr_index), ++linear_index) {

        const struct kefir_asmcmp_instruction *instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(&target->context, instr_index, &instr));
        REQUIRE_OK(kefir_hashtree_insert(mem, &allocator->internal.instruction_linear_indices,
                                         (kefir_hashtree_key_t) instr_index, (kefir_hashtree_value_t) linear_index));
        REQUIRE_OK(update_virtual_register_lifetime(target, allocator, &instr->args[0], linear_index));
        REQUIRE_OK(update_virtual_register_lifetime(target, allocator, &instr->args[1], linear_index));
        REQUIRE_OK(update_virtual_register_lifetime(target, allocator, &instr->args[2], linear_index));
    }
    return KEFIR_OK;
}

static kefir_result_t build_virtual_register_liveness_graph(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                            struct kefir_codegen_amd64_register_allocator *allocator,
                                                            const struct kefir_asmcmp_value *value,
                                                            kefir_size_t lifetime_index) {
#define UPDATE_GRAPH(_vreg)                                                                                            \
    do {                                                                                                               \
        struct kefir_codegen_amd64_register_allocation *alloc = &allocator->allocations[(_vreg)];                      \
        if (alloc->lifetime.begin == lifetime_index &&                                                                 \
            !kefir_hashtreeset_has(&allocator->internal.alive_virtual_registers,                                       \
                                   (kefir_hashtreeset_entry_t) (_vreg))) {                                             \
            struct kefir_hashtreeset_iterator iter;                                                                    \
            kefir_result_t res;                                                                                        \
            for (res = kefir_hashtreeset_iter(&allocator->internal.alive_virtual_registers, &iter); res == KEFIR_OK;   \
                 res = kefir_hashtreeset_next(&iter)) {                                                                \
                ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, other_vreg, (kefir_uptr_t) iter.entry);        \
                REQUIRE_OK(kefir_graph_new_edge(mem, &allocator->internal.liveness_graph,                              \
                                                (kefir_graph_node_id_t) other_vreg, (kefir_graph_node_id_t) (_vreg))); \
            }                                                                                                          \
            if (res != KEFIR_ITERATOR_END) {                                                                           \
                REQUIRE_OK(res);                                                                                       \
            }                                                                                                          \
            REQUIRE_OK(kefir_hashtreeset_add(mem, &allocator->internal.alive_virtual_registers,                        \
                                             (kefir_hashtreeset_entry_t) (_vreg)));                                    \
            REQUIRE_OK(kefir_graph_new_node(mem, &allocator->internal.liveness_graph, (kefir_graph_node_id_t) (_vreg), \
                                            (kefir_graph_node_value_t) 0));                                            \
        }                                                                                                              \
    } while (false)

    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT:
        case KEFIR_ASMCMP_VALUE_TYPE_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            UPDATE_GRAPH(value->vreg.index);
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    UPDATE_GRAPH(value->indirect.base.vreg);
                    break;

                case KEFIR_ASMCMP_INDIRECT_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_TEMPORARY_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX: {
            kefir_asmcmp_virtual_register_index_t vreg;
            REQUIRE_OK(kefir_asmcmp_register_stash_vreg(&target->context, value->stash_idx, &vreg));
            UPDATE_GRAPH(vreg);
        } break;
    }
#undef UPDATE_GRAPH
    return KEFIR_OK;
}

static kefir_result_t deactivate_dead_vregs(struct kefir_mem *mem,
                                            struct kefir_codegen_amd64_register_allocator *allocator,
                                            kefir_size_t linear_index) {
    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&allocator->internal.alive_virtual_registers, &iter); res == KEFIR_OK;) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg, (kefir_uptr_t) iter.entry);

        struct kefir_codegen_amd64_register_allocation *vreg_allocation = &allocator->allocations[vreg];
        if (vreg_allocation->lifetime.end <= linear_index) {
            REQUIRE_OK(kefir_hashtreeset_delete(mem, &allocator->internal.alive_virtual_registers,
                                                (kefir_hashtreeset_entry_t) vreg));

            switch (vreg_allocation->type) {
                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER:
                    // Intentionally left blank
                    break;

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SLOT:
                    REQUIRE_OK(
                        kefir_bitset_set(&allocator->internal.spill_area, vreg_allocation->spill_area_slot, false));
                    break;

                case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SPACE:
                    REQUIRE_OK(kefir_bitset_set_consecutive(&allocator->internal.spill_area,
                                                            vreg_allocation->spill_area_space.index,
                                                            vreg_allocation->spill_area_space.length, false));
                    break;
            }
            res = kefir_hashtreeset_iter(&allocator->internal.alive_virtual_registers, &iter);
        } else {
            res = kefir_hashtreeset_next(&iter);
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t build_internal_liveness_graph(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                    struct kefir_codegen_amd64_register_allocator *allocator) {
    for (kefir_asmcmp_instruction_index_t instr_index = kefir_asmcmp_context_instr_head(&target->context);
         instr_index != KEFIR_ASMCMP_INDEX_NONE;
         instr_index = kefir_asmcmp_context_instr_next(&target->context, instr_index)) {

        struct kefir_hashtree_node *linear_idx_node;
        REQUIRE_OK(kefir_hashtree_at(&allocator->internal.instruction_linear_indices,
                                     (kefir_hashtree_key_t) instr_index, &linear_idx_node));
        const kefir_size_t linear_index = linear_idx_node->value;

        REQUIRE_OK(deactivate_dead_vregs(mem, allocator, linear_index));

        const struct kefir_asmcmp_instruction *instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(&target->context, instr_index, &instr));
        REQUIRE_OK(build_virtual_register_liveness_graph(mem, target, allocator, &instr->args[0], linear_index));
        REQUIRE_OK(build_virtual_register_liveness_graph(mem, target, allocator, &instr->args[1], linear_index));
        REQUIRE_OK(build_virtual_register_liveness_graph(mem, target, allocator, &instr->args[2], linear_index));
    }
    return KEFIR_OK;
}

static kefir_result_t find_conflicting_requirements(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                    struct kefir_codegen_amd64_register_allocator *allocator,
                                                    kefir_asmcmp_virtual_register_index_t vreg) {
    struct kefir_graph_edge_iterator iter;
    kefir_result_t res;
    kefir_graph_node_id_t conflict_node;
    for (res = kefir_graph_edge_iter(&allocator->internal.liveness_graph, &iter, (kefir_graph_node_id_t) vreg,
                                     &conflict_node);
         res == KEFIR_OK; res = kefir_graph_edge_next(&iter, &conflict_node)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, other_vreg, conflict_node);

        const struct kefir_asmcmp_amd64_register_preallocation *preallocation;
        REQUIRE_OK(kefir_asmcmp_amd64_get_register_preallocation(target, other_vreg, &preallocation));
        if (preallocation != NULL && preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT) {
            REQUIRE_OK(kefir_hashtreeset_add(mem, &allocator->internal.conflicting_requirements,
                                             (kefir_hashtreeset_entry_t) preallocation->reg));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t is_register_allocated(struct kefir_codegen_amd64_register_allocator *allocator,
                                            kefir_asm_amd64_xasmgen_register_t reg, kefir_bool_t *found) {
    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res;
    for (res = kefir_hashtreeset_iter(&allocator->internal.alive_virtual_registers, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg, (kefir_uptr_t) iter.entry);

        const struct kefir_codegen_amd64_register_allocation *const alloc = &allocator->allocations[vreg];
        if (alloc->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER && alloc->direct_reg == reg) {
            *found = true;
            return KEFIR_OK;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    *found = false;
    return KEFIR_OK;
}

static kefir_result_t assign_register(struct kefir_mem *mem, struct kefir_codegen_amd64_stack_frame *stack_frame,
                                      struct kefir_codegen_amd64_register_allocator *allocator,
                                      kefir_asmcmp_virtual_register_index_t vreg_idx,
                                      kefir_asm_amd64_xasmgen_register_t reg) {
    struct kefir_codegen_amd64_register_allocation *alloc = &allocator->allocations[vreg_idx];

    alloc->type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER;
    alloc->direct_reg = reg;
    REQUIRE_OK(kefir_hashtreeset_add(mem, &allocator->used_registers, (kefir_hashtreeset_entry_t) reg));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_use_register(mem, stack_frame, reg));
    return KEFIR_OK;
}

static kefir_result_t assign_spill_index(struct kefir_codegen_amd64_register_allocator *allocator,
                                         kefir_asmcmp_virtual_register_index_t vreg_idx, kefir_size_t spill_index) {
    struct kefir_codegen_amd64_register_allocation *alloc = &allocator->allocations[vreg_idx];

    REQUIRE_OK(kefir_bitset_set(&allocator->internal.spill_area, spill_index, true));
    alloc->type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SLOT;
    alloc->spill_area_slot = spill_index;
    return KEFIR_OK;
}

static kefir_result_t assign_spill_space(struct kefir_codegen_amd64_register_allocator *allocator,
                                         kefir_asmcmp_virtual_register_index_t vreg_idx, kefir_size_t spill_index,
                                         kefir_size_t qwords) {
    struct kefir_codegen_amd64_register_allocation *alloc = &allocator->allocations[vreg_idx];

    REQUIRE_OK(kefir_bitset_set_consecutive(&allocator->internal.spill_area, spill_index, qwords, true));
    alloc->type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SPACE;
    alloc->spill_area_space.index = spill_index;
    alloc->spill_area_space.length = qwords;
    return KEFIR_OK;
}

static kefir_result_t allocate_spill_slot(struct kefir_mem *mem,
                                          struct kefir_codegen_amd64_register_allocator *allocator,
                                          struct kefir_codegen_amd64_stack_frame *stack_frame,
                                          kefir_asmcmp_virtual_register_index_t vreg_idx) {
    kefir_size_t spill_index;
    kefir_result_t res = kefir_bitset_find(&allocator->internal.spill_area, false, 0, &spill_index);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        REQUIRE_OK(assign_spill_index(allocator, vreg_idx, spill_index));
        return KEFIR_OK;
    }

    kefir_size_t num_of_slots;
    REQUIRE_OK(kefir_bitset_length(&allocator->internal.spill_area, &num_of_slots));
    REQUIRE_OK(kefir_bitset_resize(mem, &allocator->internal.spill_area, ++num_of_slots));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_ensure_spill_area(stack_frame, num_of_slots));

    REQUIRE_OK(kefir_bitset_find(&allocator->internal.spill_area, false, 0, &spill_index));
    REQUIRE_OK(assign_spill_index(allocator, vreg_idx, spill_index));
    return KEFIR_OK;
}

static kefir_result_t allocate_spill_space(struct kefir_mem *mem,
                                           struct kefir_codegen_amd64_register_allocator *allocator,
                                           struct kefir_codegen_amd64_stack_frame *stack_frame,
                                           kefir_asmcmp_virtual_register_index_t vreg_idx, kefir_size_t qwords) {
    if (qwords == 0) {
        REQUIRE_OK(assign_spill_space(allocator, vreg_idx, 0, 0));
        return KEFIR_OK;
    }

    kefir_size_t spill_index;
    kefir_result_t res = kefir_bitset_find_consecutive(&allocator->internal.spill_area, false, qwords, 0, &spill_index);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        REQUIRE_OK(assign_spill_space(allocator, vreg_idx, spill_index, qwords));
        return KEFIR_OK;
    }

    kefir_size_t num_of_slots;
    REQUIRE_OK(kefir_bitset_length(&allocator->internal.spill_area, &num_of_slots));
    num_of_slots += qwords;
    REQUIRE_OK(kefir_bitset_resize(mem, &allocator->internal.spill_area, num_of_slots));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_ensure_spill_area(stack_frame, num_of_slots));

    REQUIRE_OK(kefir_bitset_find_consecutive(&allocator->internal.spill_area, false, qwords, 0, &spill_index));
    REQUIRE_OK(assign_spill_space(allocator, vreg_idx, spill_index, qwords));
    return KEFIR_OK;
}

static kefir_result_t allocate_register_impl(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                             struct kefir_codegen_amd64_stack_frame *stack_frame,
                                             struct kefir_codegen_amd64_register_allocator *allocator,
                                             kefir_asmcmp_virtual_register_index_t vreg_idx) {
    const struct kefir_asmcmp_virtual_register *vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&target->context, vreg_idx, &vreg));
    struct kefir_codegen_amd64_register_allocation *alloc = &allocator->allocations[vreg_idx];
    REQUIRE(alloc->type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED, KEFIR_OK);

    REQUIRE_OK(kefir_hashtreeset_clean(mem, &allocator->internal.conflicting_requirements));
    REQUIRE_OK(find_conflicting_requirements(mem, target, allocator, vreg_idx));
    REQUIRE_OK(
        kefir_hashtreeset_add(mem, &allocator->internal.alive_virtual_registers, (kefir_hashtreeset_entry_t) vreg_idx));

    const struct kefir_asmcmp_amd64_register_preallocation *preallocation;
    REQUIRE_OK(kefir_asmcmp_amd64_get_register_preallocation(target, vreg_idx, &preallocation));

    kefir_bool_t found_alive_reg;
    switch (vreg->type) {
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unspecified virtual register type");

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE:
            if (preallocation != NULL && preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT) {
                REQUIRE_OK(is_register_allocated(allocator, preallocation->reg, &found_alive_reg));
                REQUIRE(!found_alive_reg,
                        KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                                        "Unable to satisfy amd64 register preallocation requirement"));

                REQUIRE_OK(assign_register(mem, stack_frame, allocator, vreg_idx, preallocation->reg));
                return KEFIR_OK;
            }

            if (preallocation != NULL && preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_HINT &&
                !kefir_hashtreeset_has(&allocator->internal.conflicting_requirements,
                                       (kefir_hashtreeset_entry_t) preallocation->reg)) {

                REQUIRE_OK(is_register_allocated(allocator, preallocation->reg, &found_alive_reg));
                if (!found_alive_reg) {
                    REQUIRE_OK(assign_register(mem, stack_frame, allocator, vreg_idx, preallocation->reg));
                    return KEFIR_OK;
                }
            }

            if (preallocation != NULL && preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_SAME_AS &&
                !kefir_hashtreeset_has(&allocator->internal.alive_virtual_registers,
                                       (kefir_hashtreeset_entry_t) preallocation->vreg)) {
                const struct kefir_codegen_amd64_register_allocation *other_alloc =
                    &allocator->allocations[preallocation->vreg];
                switch (other_alloc->type) {
                    case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED:
                    case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER:
                    case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SPACE:
                        // Intentionally left blank
                        break;

                    case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
                        REQUIRE_OK(is_register_allocated(allocator, other_alloc->direct_reg, &found_alive_reg));
                        if (!kefir_hashtreeset_has(&allocator->internal.conflicting_requirements,
                                                   (kefir_hashtreeset_entry_t) other_alloc->direct_reg) &&
                            !found_alive_reg) {
                            REQUIRE_OK(assign_register(mem, stack_frame, allocator, vreg_idx, other_alloc->direct_reg));
                            return KEFIR_OK;
                        }
                        break;

                    case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_SLOT: {
                        kefir_bool_t spill_slot_empty;
                        REQUIRE_OK(kefir_bitset_get(&allocator->internal.spill_area, other_alloc->spill_area_slot,
                                                    &spill_slot_empty));
                        if (!spill_slot_empty) {
                            REQUIRE_OK(assign_spill_index(allocator, vreg_idx, other_alloc->spill_area_slot));
                            return KEFIR_OK;
                        }
                    } break;
                }
            }

            for (kefir_size_t i = 0; i < NUM_OF_AMD64_GENERAL_PURPOSE_REGS; i++) {
                kefir_asm_amd64_xasmgen_register_t candidate = allocator->internal.gp_register_allocation_order[i];
                if (!kefir_hashtreeset_has(&allocator->internal.conflicting_requirements,
                                           (kefir_hashtreeset_entry_t) candidate)) {

                    REQUIRE_OK(is_register_allocated(allocator, candidate, &found_alive_reg));
                    if (!found_alive_reg) {
                        REQUIRE_OK(assign_register(mem, stack_frame, allocator, vreg_idx, candidate));
                        return KEFIR_OK;
                    }
                }
            }

            REQUIRE_OK(allocate_spill_slot(mem, allocator, stack_frame, vreg_idx));
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_SPILL_SPACE_SLOT:
            REQUIRE_OK(allocate_spill_slot(mem, allocator, stack_frame, vreg_idx));
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_SPILL_SPACE_ALLOCATION:
            REQUIRE_OK(allocate_spill_space(mem, allocator, stack_frame, vreg_idx,
                                            vreg->parameters.spill_space_allocation_length));
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_MEMORY_POINTER: {
            struct kefir_codegen_amd64_register_allocation *alloc = &allocator->allocations[vreg_idx];
            REQUIRE(vreg->parameters.memory.base_reg != (kefir_asmcmp_physical_register_index_t) -1ll,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unassigned virtual register memory location"));
            alloc->type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER;
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t allocate_register(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                        struct kefir_codegen_amd64_stack_frame *stack_frame,
                                        struct kefir_codegen_amd64_register_allocator *allocator,
                                        const struct kefir_asmcmp_value *value) {
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT:
        case KEFIR_ASMCMP_VALUE_TYPE_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER:
            REQUIRE_OK(allocate_register_impl(mem, target, stack_frame, allocator, value->vreg.index));
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS:
                    REQUIRE_OK(allocate_register_impl(mem, target, stack_frame, allocator, value->indirect.base.vreg));
                    break;

                case KEFIR_ASMCMP_INDIRECT_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_VAR_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_TEMPORARY_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX: {
            kefir_asmcmp_virtual_register_index_t vreg;
            REQUIRE_OK(kefir_asmcmp_register_stash_vreg(&target->context, value->stash_idx, &vreg));
            REQUIRE_OK(allocate_register_impl(mem, target, stack_frame, allocator, vreg));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t update_stash(struct kefir_asmcmp_amd64 *target,
                                   struct kefir_codegen_amd64_register_allocator *allocator,
                                   const struct kefir_asmcmp_instruction *instr) {
    ASSIGN_DECL_CAST(kefir_asmcmp_stash_index_t, stash_idx, instr->args[0].stash_idx);

    kefir_size_t qwords = 0;

    kefir_asmcmp_instruction_index_t liveness_idx;
    REQUIRE_OK(kefir_asmcmp_register_stash_liveness_index(&target->context, stash_idx, &liveness_idx));

    struct kefir_hashtreeset_iterator iter;
    kefir_result_t res = KEFIR_OK;
    for (res = kefir_hashtreeset_iter(&allocator->internal.alive_virtual_registers, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg, (kefir_uptr_t) iter.entry);

        const struct kefir_codegen_amd64_register_allocation *reg_alloc = &allocator->allocations[vreg];
        if (reg_alloc->type != KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER) {
            continue;
        }

        kefir_bool_t contains_phreg;
        kefir_result_t res =
            kefir_asmcmp_register_stash_has(&target->context, stash_idx, reg_alloc->direct_reg, &contains_phreg);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        if (((liveness_idx != KEFIR_ASMCMP_INDEX_NONE &&
              !(reg_alloc->lifetime.begin <= liveness_idx && reg_alloc->lifetime.end > liveness_idx))) ||
            !contains_phreg) {
            continue;
        }

        qwords++;
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_asmcmp_virtual_register_index_t spill_vreg;
    REQUIRE_OK(kefir_asmcmp_register_stash_vreg(&target->context, stash_idx, &spill_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_set_spill_space_size(&target->context, spill_vreg, qwords));

    UNUSED(qwords);
    return KEFIR_OK;
}

static kefir_result_t allocate_registers(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                         struct kefir_codegen_amd64_stack_frame *stack_frame,
                                         struct kefir_codegen_amd64_register_allocator *allocator) {
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &allocator->internal.alive_virtual_registers));

    for (kefir_asmcmp_instruction_index_t instr_index = kefir_asmcmp_context_instr_head(&target->context);
         instr_index != KEFIR_ASMCMP_INDEX_NONE;
         instr_index = kefir_asmcmp_context_instr_next(&target->context, instr_index)) {

        struct kefir_hashtree_node *linear_idx_node;
        REQUIRE_OK(kefir_hashtree_at(&allocator->internal.instruction_linear_indices,
                                     (kefir_hashtree_key_t) instr_index, &linear_idx_node));
        const kefir_size_t linear_index = linear_idx_node->value;

        const struct kefir_asmcmp_instruction *instr;
        REQUIRE_OK(deactivate_dead_vregs(mem, allocator, linear_index));
        REQUIRE_OK(kefir_asmcmp_context_instr_at(&target->context, instr_index, &instr));

        if (instr->opcode == KEFIR_ASMCMP_AMD64_OPCODE(stash_activate)) {
            REQUIRE_OK(update_stash(target, allocator, instr));
        }

        REQUIRE_OK(allocate_register(mem, target, stack_frame, allocator, &instr->args[0]));
        REQUIRE_OK(allocate_register(mem, target, stack_frame, allocator, &instr->args[1]));
        REQUIRE_OK(allocate_register(mem, target, stack_frame, allocator, &instr->args[2]));
    }
    return KEFIR_OK;
}

static kefir_result_t is_callee_preserved_reg(kefir_abi_amd64_variant_t variant, kefir_asm_amd64_xasmgen_register_t reg,
                                              kefir_bool_t *preserved) {
    const kefir_size_t num_of_regs = kefir_abi_amd64_num_of_callee_preserved_general_purpose_registers(variant);
    kefir_asm_amd64_xasmgen_register_t preserved_reg;
    for (kefir_size_t i = 0; i < num_of_regs; i++) {
        REQUIRE_OK(kefir_abi_amd64_get_callee_preserved_general_purpose_register(variant, i, &preserved_reg));
        if (preserved_reg == reg) {
            *preserved = true;
            return KEFIR_OK;
        }
    }

    *preserved = false;
    return KEFIR_OK;
}

static kefir_result_t abi_register_comparator(void *ptr1, void *ptr2, kefir_int_t *cmp, void *payload) {
    UNUSED(payload);
    REQUIRE(ptr1 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid first register pointer"));
    REQUIRE(ptr2 != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid second register pointer"));
    REQUIRE(cmp != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to comparison result"));
    REQUIRE(payload != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid comparator payload"));
    ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t *, reg1, ptr1);
    ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t *, reg2, ptr2);
    ASSIGN_DECL_CAST(struct kefir_asmcmp_amd64 *, target, payload);

    kefir_bool_t preserved1, preserved2;
    REQUIRE_OK(is_callee_preserved_reg(target->abi_variant, *reg1, &preserved1));
    REQUIRE_OK(is_callee_preserved_reg(target->abi_variant, *reg2, &preserved2));

    if (!preserved1 && preserved2) {
        *cmp = -1;
    } else if (preserved1 && !preserved2) {
        *cmp = 1;
    } else if (*reg1 < *reg2) {
        *cmp = -1;
    } else if (*reg1 == *reg2) {
        *cmp = 0;
    } else {
        *cmp = 1;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_register_allocator_run(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                                          struct kefir_codegen_amd64_stack_frame *stack_frame,
                                                          struct kefir_codegen_amd64_register_allocator *allocator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(allocator->allocations == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided amd64 register allocator has already been ran"));

    kefir_size_t num_of_vregs;
    REQUIRE_OK(kefir_asmcmp_number_of_virtual_registers(&target->context, &num_of_vregs));
    allocator->allocations = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_amd64_register_allocation) * num_of_vregs);
    REQUIRE(allocator->allocations != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 register allocations"));
    for (kefir_size_t i = 0; i < num_of_vregs; i++) {
        allocator->allocations[i].type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED;
        allocator->allocations[i].lifetime.begin = KEFIR_ASMCMP_INDEX_NONE;
        allocator->allocations[i].lifetime.end = KEFIR_ASMCMP_INDEX_NONE;
    }
    allocator->num_of_vregs = num_of_vregs;

    allocator->internal.gp_register_allocation_order =
        KEFIR_MALLOC(mem, sizeof(kefir_asm_amd64_xasmgen_register_t) * NUM_OF_AMD64_GENERAL_PURPOSE_REGS);
    REQUIRE(allocator->internal.gp_register_allocation_order != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 register allocation order"));
    memcpy(allocator->internal.gp_register_allocation_order, AMD64_GENERAL_PURPOSE_REGS,
           sizeof(kefir_asm_amd64_xasmgen_register_t) * NUM_OF_AMD64_GENERAL_PURPOSE_REGS);
    REQUIRE_OK(kefir_mergesort(mem, allocator->internal.gp_register_allocation_order,
                               sizeof(kefir_asm_amd64_xasmgen_register_t), NUM_OF_AMD64_GENERAL_PURPOSE_REGS,
                               abi_register_comparator, target));
    allocator->internal.num_of_gp_registers = NUM_OF_AMD64_GENERAL_PURPOSE_REGS;

    REQUIRE_OK(calculate_lifetimes(mem, target, allocator));
    REQUIRE_OK(build_internal_liveness_graph(mem, target, allocator));
    REQUIRE_OK(allocate_registers(mem, target, stack_frame, allocator));
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_register_allocation_of(
    const struct kefir_codegen_amd64_register_allocator *allocator, kefir_asmcmp_virtual_register_index_t vreg_idx,
    const struct kefir_codegen_amd64_register_allocation **allocation_ptr) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(vreg_idx < allocator->num_of_vregs,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Requested virtual register is out of allocator bounds"));
    REQUIRE(allocation_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register allocation"));

    *allocation_ptr = &allocator->allocations[vreg_idx];
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_register_allocator_linear_position_of(
    const struct kefir_codegen_amd64_register_allocator *allocator, kefir_asmcmp_instruction_index_t instr_idx,
    kefir_size_t *linear_pos_ptr) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(linear_pos_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to linear position"));

    struct kefir_hashtree_node *node;
    kefir_result_t res =
        kefir_hashtree_at(&allocator->internal.instruction_linear_indices, (kefir_hashtree_key_t) instr_idx, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find linear position of requested instruction index");
    }
    REQUIRE_OK(res);

    *linear_pos_ptr = (kefir_size_t) node->value;
    return KEFIR_OK;
}

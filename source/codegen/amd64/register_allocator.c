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
    allocator->internal.register_allocation_order = NULL;
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
    if (allocator->internal.register_allocation_order != NULL) {
        KEFIR_FREE(mem, allocator->internal.register_allocation_order);
    }
    memset(allocator, 0, sizeof(struct kefir_codegen_amd64_register_allocator));
    return KEFIR_OK;
}

static kefir_result_t update_virtual_register_lifetime(struct kefir_codegen_amd64_register_allocator *allocator,
                                                       const struct kefir_asmcmp_value *value,
                                                       kefir_size_t lifetime_index) {
    REQUIRE(value->type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, KEFIR_OK);

    kefir_asmcmp_virtual_register_index_t vreg = value->vreg.index;
    struct kefir_codegen_amd64_register_allocation *alloc = &allocator->allocations[vreg];
    if (alloc->lifetime.begin == KEFIR_ASMCMP_INDEX_NONE) {
        alloc->lifetime.begin = lifetime_index;
        alloc->lifetime.end = lifetime_index;
    } else {
        alloc->lifetime.begin = MIN(alloc->lifetime.begin, lifetime_index);
        alloc->lifetime.end = MAX(alloc->lifetime.end, lifetime_index);
    }
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
        REQUIRE_OK(update_virtual_register_lifetime(allocator, &instr->args[0], linear_index));
        REQUIRE_OK(update_virtual_register_lifetime(allocator, &instr->args[1], linear_index));
        REQUIRE_OK(update_virtual_register_lifetime(allocator, &instr->args[2], linear_index));
    }
    return KEFIR_OK;
}

static kefir_result_t build_virtual_register_liveness_graph(struct kefir_mem *mem,
                                                            struct kefir_codegen_amd64_register_allocator *allocator,
                                                            const struct kefir_asmcmp_value *value,
                                                            kefir_size_t lifetime_index) {
    REQUIRE(value->type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, KEFIR_OK);

    kefir_asmcmp_virtual_register_index_t vreg = value->vreg.index;
    struct kefir_codegen_amd64_register_allocation *alloc = &allocator->allocations[vreg];
    if (alloc->lifetime.begin == lifetime_index &&
        !kefir_hashtreeset_has(&allocator->internal.alive_virtual_registers, (kefir_hashtreeset_entry_t) vreg)) {
        struct kefir_hashtreeset_iterator iter;
        kefir_result_t res;
        for (res = kefir_hashtreeset_iter(&allocator->internal.alive_virtual_registers, &iter); res == KEFIR_OK;
             res = kefir_hashtreeset_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, other_vreg, (kefir_uptr_t) iter.entry);
            REQUIRE_OK(kefir_graph_new_edge(mem, &allocator->internal.liveness_graph,
                                            (kefir_graph_node_id_t) other_vreg, (kefir_graph_node_id_t) vreg));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(
            kefir_hashtreeset_add(mem, &allocator->internal.alive_virtual_registers, (kefir_hashtreeset_entry_t) vreg));
        REQUIRE_OK(kefir_graph_new_node(mem, &allocator->internal.liveness_graph, (kefir_graph_node_id_t) vreg,
                                        (kefir_graph_node_value_t) 0));
    }
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
                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE:
                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER:
                    // Intentionally left blank
                    break;

                case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA:
                    REQUIRE_OK(
                        kefir_bitset_set(&allocator->internal.spill_area, vreg_allocation->spill_area_index, false));
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
        REQUIRE_OK(build_virtual_register_liveness_graph(mem, allocator, &instr->args[0], linear_index));
        REQUIRE_OK(build_virtual_register_liveness_graph(mem, allocator, &instr->args[1], linear_index));
        REQUIRE_OK(build_virtual_register_liveness_graph(mem, allocator, &instr->args[2], linear_index));
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
        if (alloc->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER && alloc->direct_reg == reg) {
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

static kefir_result_t assign_register(struct kefir_mem *mem, struct kefir_codegen_amd64_register_allocator *allocator,
                                      kefir_asmcmp_virtual_register_index_t vreg_idx,
                                      kefir_asm_amd64_xasmgen_register_t reg) {
    struct kefir_codegen_amd64_register_allocation *alloc = &allocator->allocations[vreg_idx];

    alloc->type = KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER;
    alloc->direct_reg = reg;
    REQUIRE_OK(kefir_hashtreeset_add(mem, &allocator->used_registers, (kefir_hashtreeset_entry_t) reg));
    return KEFIR_OK;
}

static kefir_result_t assign_spill_index(struct kefir_codegen_amd64_register_allocator *allocator,
                                         kefir_asmcmp_virtual_register_index_t vreg_idx, kefir_size_t spill_index) {
    struct kefir_codegen_amd64_register_allocation *alloc = &allocator->allocations[vreg_idx];

    REQUIRE_OK(kefir_bitset_set(&allocator->internal.spill_area, spill_index, true));
    alloc->type = KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA;
    alloc->spill_area_index = spill_index;
    return KEFIR_OK;
}

static kefir_result_t allocate_spill_slot(struct kefir_mem *mem,
                                          struct kefir_codegen_amd64_register_allocator *allocator,
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
    REQUIRE_OK(kefir_bitset_resize(mem, &allocator->internal.spill_area, num_of_slots + 1));

    REQUIRE_OK(kefir_bitset_find(&allocator->internal.spill_area, false, 0, &spill_index));
    REQUIRE_OK(assign_spill_index(allocator, vreg_idx, spill_index));
    return KEFIR_OK;
}

static kefir_result_t allocate_register(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                        struct kefir_codegen_amd64_register_allocator *allocator,
                                        const struct kefir_asmcmp_value *value) {
    REQUIRE(value->type == KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER, KEFIR_OK);

    kefir_asmcmp_virtual_register_index_t vreg_idx = value->vreg.index;
    const struct kefir_asmcmp_virtual_register *vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&target->context, vreg_idx, &vreg));
    struct kefir_codegen_amd64_register_allocation *alloc = &allocator->allocations[vreg_idx];
    REQUIRE(alloc->type == KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE, KEFIR_OK);

    REQUIRE_OK(kefir_hashtreeset_clean(mem, &allocator->internal.conflicting_requirements));
    REQUIRE_OK(find_conflicting_requirements(mem, target, allocator, vreg_idx));
    REQUIRE_OK(
        kefir_hashtreeset_add(mem, &allocator->internal.alive_virtual_registers, (kefir_hashtreeset_entry_t) vreg_idx));

    const struct kefir_asmcmp_amd64_register_preallocation *preallocation;
    REQUIRE_OK(kefir_asmcmp_amd64_get_register_preallocation(target, vreg_idx, &preallocation));

    kefir_bool_t found_alive_reg;
    switch (vreg->type) {
        case KEFIR_ASMCMP_REGISTER_GENERAL_PURPOSE:
            if (preallocation != NULL && preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT) {
                REQUIRE_OK(is_register_allocated(allocator, preallocation->reg, &found_alive_reg));
                REQUIRE(!found_alive_reg,
                        KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                                        "Unable to satisfy amd64 register preallocation requirement"));

                REQUIRE_OK(assign_register(mem, allocator, vreg_idx, preallocation->reg));
                return KEFIR_OK;
            }

            if (preallocation != NULL && preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_HINT &&
                !kefir_hashtreeset_has(&allocator->internal.conflicting_requirements,
                                       (kefir_hashtreeset_entry_t) preallocation->reg)) {

                REQUIRE_OK(is_register_allocated(allocator, preallocation->reg, &found_alive_reg));
                if (!found_alive_reg) {
                    REQUIRE_OK(assign_register(mem, allocator, vreg_idx, preallocation->reg));
                    return KEFIR_OK;
                }
            }

            if (preallocation && preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_SAME_AS &&
                !kefir_hashtreeset_has(&allocator->internal.alive_virtual_registers,
                                       (kefir_hashtreeset_entry_t) preallocation->vreg)) {
                const struct kefir_codegen_amd64_register_allocation *other_alloc =
                    &allocator->allocations[preallocation->vreg];
                switch (other_alloc->type) {
                    case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE:
                        // Intentionally left blank
                        break;

                    case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_REGISTER:
                        REQUIRE_OK(is_register_allocated(allocator, other_alloc->direct_reg, &found_alive_reg));
                        if (!kefir_hashtreeset_has(&allocator->internal.conflicting_requirements,
                                                   (kefir_hashtreeset_entry_t) other_alloc->direct_reg) &&
                            !found_alive_reg) {
                            REQUIRE_OK(assign_register(mem, allocator, vreg_idx, other_alloc->direct_reg));
                            return KEFIR_OK;
                        }
                        break;

                    case KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_SPILL_AREA: {
                        kefir_bool_t spill_slot_empty;
                        REQUIRE_OK(kefir_bitset_get(&allocator->internal.spill_area, other_alloc->spill_area_index,
                                                    &spill_slot_empty));
                        if (!spill_slot_empty) {
                            REQUIRE_OK(assign_spill_index(allocator, vreg_idx, other_alloc->spill_area_index));
                            return KEFIR_OK;
                        }
                    } break;
                }
            }

            for (kefir_size_t i = 0; i < NUM_OF_AMD64_GENERAL_PURPOSE_REGS; i++) {
                kefir_asm_amd64_xasmgen_register_t candidate = allocator->internal.register_allocation_order[i];
                if (!kefir_hashtreeset_has(&allocator->internal.conflicting_requirements,
                                           (kefir_hashtreeset_entry_t) candidate)) {

                    REQUIRE_OK(is_register_allocated(allocator, candidate, &found_alive_reg));
                    if (!found_alive_reg) {
                        REQUIRE_OK(assign_register(mem, allocator, vreg_idx, candidate));
                        return KEFIR_OK;
                    }
                }
            }

            REQUIRE_OK(allocate_spill_slot(mem, allocator, vreg_idx));
            break;
    }

    return KEFIR_OK;
}

static kefir_result_t allocate_registers(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
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
        REQUIRE_OK(allocate_register(mem, target, allocator, &instr->args[0]));
        REQUIRE_OK(allocate_register(mem, target, allocator, &instr->args[1]));
        REQUIRE_OK(allocate_register(mem, target, allocator, &instr->args[2]));
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
                                                          struct kefir_codegen_amd64_register_allocator *allocator) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(target != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 asmcmp"));
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(allocator->allocations == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Provided amd64 register allocator has already been ran"));

    kefir_size_t num_of_vregs;
    REQUIRE_OK(kefir_asmcmp_number_of_virtual_registers(&target->context, &num_of_vregs));
    allocator->allocations = KEFIR_MALLOC(mem, sizeof(struct kefir_codegen_amd64_register_allocation) * num_of_vregs);
    REQUIRE(allocator->allocations != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 register allocations"));
    for (kefir_size_t i = 0; i < num_of_vregs; i++) {
        allocator->allocations[i].type = KEFIR_CODEGEN_AMD64_REGISTER_ALLOCATION_NONE;
        allocator->allocations[i].lifetime.begin = KEFIR_ASMCMP_INDEX_NONE;
        allocator->allocations[i].lifetime.end = KEFIR_ASMCMP_INDEX_NONE;
    }
    allocator->num_of_vregs = num_of_vregs;

    allocator->internal.register_allocation_order =
        KEFIR_MALLOC(mem, sizeof(kefir_asm_amd64_xasmgen_register_t) * NUM_OF_AMD64_GENERAL_PURPOSE_REGS);
    REQUIRE(allocator->internal.register_allocation_order != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 register allocation order"));
    memcpy(allocator->internal.register_allocation_order, AMD64_GENERAL_PURPOSE_REGS,
           sizeof(kefir_asm_amd64_xasmgen_register_t) * NUM_OF_AMD64_GENERAL_PURPOSE_REGS);
    REQUIRE_OK(kefir_mergesort(mem, allocator->internal.register_allocation_order,
                               sizeof(kefir_asm_amd64_xasmgen_register_t), NUM_OF_AMD64_GENERAL_PURPOSE_REGS,
                               abi_register_comparator, target));

    REQUIRE_OK(calculate_lifetimes(mem, target, allocator));
    REQUIRE_OK(build_internal_liveness_graph(mem, target, allocator));
    REQUIRE_OK(allocate_registers(mem, target, allocator));
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

kefir_result_t kefir_codegen_amd64_register_allocator_is_register_used(
    const struct kefir_codegen_amd64_register_allocator *allocator, kefir_asm_amd64_xasmgen_register_t reg,
    kefir_bool_t *flag) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(flag != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    *flag = kefir_hashtreeset_has(&allocator->used_registers, (kefir_hashtreeset_entry_t) reg);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_register_allocator_num_of_spill_slots(
    const struct kefir_codegen_amd64_register_allocator *allocator, kefir_size_t *slots) {
    REQUIRE(allocator != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(slots != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to number of spill slots"));

    REQUIRE_OK(kefir_bitset_length(&allocator->internal.spill_area, slots));
    return KEFIR_OK;
}

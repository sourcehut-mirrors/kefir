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

#include "kefir/codegen/amd64/xregalloc.h"
#include "kefir/target/abi/util.h"
#include "kefir/target/abi/amd64/function.h"
#include "kefir/core/sort.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct virtual_register_allocation_order_entry {
    struct kefir_hashtreeset virtual_registers;
};

static const kefir_asm_amd64_xasmgen_register_t AMD64_GENERAL_PURPOSE_REGS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_RAX, KEFIR_AMD64_XASMGEN_REGISTER_RBX, KEFIR_AMD64_XASMGEN_REGISTER_RCX,
    KEFIR_AMD64_XASMGEN_REGISTER_RDX, KEFIR_AMD64_XASMGEN_REGISTER_RSI, KEFIR_AMD64_XASMGEN_REGISTER_RDI,
    KEFIR_AMD64_XASMGEN_REGISTER_R8,  KEFIR_AMD64_XASMGEN_REGISTER_R9,  KEFIR_AMD64_XASMGEN_REGISTER_R10,
    KEFIR_AMD64_XASMGEN_REGISTER_R11, KEFIR_AMD64_XASMGEN_REGISTER_R12, KEFIR_AMD64_XASMGEN_REGISTER_R13,
    KEFIR_AMD64_XASMGEN_REGISTER_R14, KEFIR_AMD64_XASMGEN_REGISTER_R15};

static const kefir_size_t NUM_OF_AMD64_GENERAL_PURPOSE_REGS =
    sizeof(AMD64_GENERAL_PURPOSE_REGS) / sizeof(AMD64_GENERAL_PURPOSE_REGS[0]);

static const kefir_asm_amd64_xasmgen_register_t AMD64_FLOATING_POINT_REGS[] = {
    KEFIR_AMD64_XASMGEN_REGISTER_XMM0,  KEFIR_AMD64_XASMGEN_REGISTER_XMM1,  KEFIR_AMD64_XASMGEN_REGISTER_XMM2,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM3,  KEFIR_AMD64_XASMGEN_REGISTER_XMM4,  KEFIR_AMD64_XASMGEN_REGISTER_XMM5,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM6,  KEFIR_AMD64_XASMGEN_REGISTER_XMM7,  KEFIR_AMD64_XASMGEN_REGISTER_XMM8,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM9,  KEFIR_AMD64_XASMGEN_REGISTER_XMM10, KEFIR_AMD64_XASMGEN_REGISTER_XMM11,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM12, KEFIR_AMD64_XASMGEN_REGISTER_XMM13, KEFIR_AMD64_XASMGEN_REGISTER_XMM14,
    KEFIR_AMD64_XASMGEN_REGISTER_XMM15};

static const kefir_size_t NUM_OF_AMD64_FLOATING_POINT_REGS =
    sizeof(AMD64_FLOATING_POINT_REGS) / sizeof(AMD64_FLOATING_POINT_REGS[0]);

struct virtual_block_data {
    struct virtual_block_data *parent;
    kefir_codegen_amd64_xregalloc_virtual_block_id_t virtual_block_id;
    struct kefir_bucketset virtual_registers;
};

static kefir_result_t virtual_block_free(struct kefir_mem *mem, struct kefir_hashtree *tree, kefir_hashtree_key_t key,
                                         kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct virtual_block_data *, data, value);

    REQUIRE_OK(kefir_bucketset_free(mem, &data->virtual_registers));
    KEFIR_FREE(mem, data);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_xregalloc_init(struct kefir_codegen_amd64_xregalloc *xregalloc) {
    REQUIRE(xregalloc != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to amd64 register allocator"));

    xregalloc->linearized_code = NULL;
    xregalloc->linear_code_length = 0;
    xregalloc->virtual_registers = NULL;
    xregalloc->virtual_register_length = 0;
    xregalloc->used_slots = 0;
    REQUIRE_OK(kefir_hashtreeset_init(&xregalloc->used_registers, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&xregalloc->virtual_blocks, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&xregalloc->virtual_blocks, virtual_block_free, NULL));

    xregalloc->available_registers.general_purpose_registers = NULL;
    xregalloc->available_registers.num_of_general_purpose_registers = 0;
    xregalloc->available_registers.floating_point_registers = NULL;
    xregalloc->available_registers.num_of_floating_point_registers = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_xregalloc_free(struct kefir_mem *mem,
                                                  struct kefir_codegen_amd64_xregalloc *xregalloc) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(xregalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));

    for (kefir_size_t i = 0; i < xregalloc->virtual_register_length; i++) {
        REQUIRE_OK(kefir_bucketset_free(mem, &xregalloc->virtual_registers[i].interference));
        REQUIRE_OK(kefir_bucketset_free(mem, &xregalloc->virtual_registers[i].virtual_blocks));
    }
    REQUIRE_OK(kefir_hashtree_free(mem, &xregalloc->virtual_blocks));
    REQUIRE_OK(kefir_hashtreeset_free(mem, &xregalloc->used_registers));
    KEFIR_FREE(mem, xregalloc->linearized_code);
    KEFIR_FREE(mem, xregalloc->virtual_registers);
    KEFIR_FREE(mem, xregalloc->available_registers.general_purpose_registers);
    KEFIR_FREE(mem, xregalloc->available_registers.floating_point_registers);
    memset(xregalloc, 0, sizeof(struct kefir_codegen_amd64_xregalloc));
    return KEFIR_OK;
}

struct do_allocation_state {
    struct kefir_hashtree allocation_order;
    struct kefir_hashtreeset active_registers;
    struct kefir_hashtreeset active_hints;
    struct kefir_bitset active_spill_area;
    struct kefir_bitset active_spill_area_hints;
    struct kefir_hashtree stashes;
    struct kefir_hashtree preserve_vreg_locations;
    struct virtual_block_data *base_virtual_block;
    struct virtual_block_data *current_virtual_block;
};

static kefir_result_t add_virtual_register_to_block(struct kefir_mem *mem, const struct kefir_asmcmp_context *context,
                                                    struct virtual_block_data *block_data,
                                                    kefir_asmcmp_virtual_register_index_t vreg_idx,
                                                    struct kefir_codegen_amd64_xregalloc_virtual_register *vreg) {
    const struct kefir_asmcmp_virtual_register *asmcmp_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(context, vreg_idx, &asmcmp_vreg));
    REQUIRE(asmcmp_vreg->type != KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_INTEGER &&
                asmcmp_vreg->type != KEFIR_ASMCMP_VIRTUAL_REGISTER_LOCAL_VARIABLE,
            KEFIR_OK);
    if (asmcmp_vreg->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR) {
        REQUIRE_OK(add_virtual_register_to_block(mem, context, block_data,
                                                 asmcmp_vreg->parameters.pair.virtual_registers[0], vreg));
        REQUIRE_OK(add_virtual_register_to_block(mem, context, block_data,
                                                 asmcmp_vreg->parameters.pair.virtual_registers[1], vreg));
    }
    REQUIRE_OK(kefir_bucketset_add(mem, &block_data->virtual_registers, (kefir_bucketset_entry_t) vreg_idx));
    for (; block_data != NULL; block_data = block_data->parent) {
        REQUIRE_OK(
            kefir_bucketset_add(mem, &vreg->virtual_blocks, (kefir_bucketset_entry_t) block_data->virtual_block_id));
    }
    return KEFIR_OK;
}

static kefir_result_t scan_virtual_register(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *target,
                                            struct kefir_codegen_amd64_xregalloc *xregalloc,
                                            const struct kefir_asmcmp_value *value, kefir_size_t linear_index,
                                            struct virtual_block_data *block_data) {
    switch (value->type) {
        case KEFIR_ASMCMP_VALUE_TYPE_NONE:
        case KEFIR_ASMCMP_VALUE_TYPE_INTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_UINTEGER:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_INTERNAL:
        case KEFIR_ASMCMP_VALUE_TYPE_RIP_INDIRECT_EXTERNAL:
        case KEFIR_ASMCMP_VALUE_TYPE_INTERNAL_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_EXTERNAL_LABEL:
        case KEFIR_ASMCMP_VALUE_TYPE_X87:
        case KEFIR_ASMCMP_VALUE_TYPE_PHYSICAL_REGISTER:
        case KEFIR_ASMCMP_VALUE_TYPE_INLINE_ASSEMBLY_INDEX:
            // Intentionally left blank
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_VIRTUAL_REGISTER: {
            struct kefir_codegen_amd64_xregalloc_virtual_register *vreg =
                &xregalloc->virtual_registers[value->vreg.index];
            if (vreg->lifetime.begin == KEFIR_CODEGEN_AMD64_XREGALLOC_UNDEFINED) {
                vreg->lifetime.begin = linear_index;
                vreg->lifetime.end = linear_index;
            } else {
                vreg->lifetime.begin = MIN(vreg->lifetime.begin, linear_index);
                vreg->lifetime.end = MAX(vreg->lifetime.end, linear_index);
            }
            REQUIRE_OK(add_virtual_register_to_block(mem, &target->context, block_data, value->vreg.index, vreg));
        } break;

        case KEFIR_ASMCMP_VALUE_TYPE_INDIRECT:
            switch (value->indirect.type) {
                case KEFIR_ASMCMP_INDIRECT_VIRTUAL_BASIS: {
                    struct kefir_codegen_amd64_xregalloc_virtual_register *vreg =
                        &xregalloc->virtual_registers[value->indirect.base.vreg];
                    if (vreg->lifetime.begin == KEFIR_CODEGEN_AMD64_XREGALLOC_UNDEFINED) {
                        vreg->lifetime.begin = linear_index;
                        vreg->lifetime.end = linear_index;
                    } else {
                        vreg->lifetime.begin = MIN(vreg->lifetime.begin, linear_index);
                        vreg->lifetime.end = MAX(vreg->lifetime.end, linear_index);
                    }
                    REQUIRE_OK(add_virtual_register_to_block(mem, &target->context, block_data,
                                                             value->indirect.base.vreg, vreg));
                } break;

                case KEFIR_ASMCMP_INDIRECT_INTERNAL_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_EXTERNAL_LABEL_BASIS:
                case KEFIR_ASMCMP_INDIRECT_LOCAL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_SPILL_AREA_BASIS:
                case KEFIR_ASMCMP_INDIRECT_PHYSICAL_BASIS:
                    // Intentionally left blank
                    break;
            }
            break;

        case KEFIR_ASMCMP_VALUE_TYPE_STASH_INDEX: {
            kefir_asmcmp_virtual_register_index_t vreg_idx;
            REQUIRE_OK(kefir_asmcmp_register_stash_vreg(&target->context, value->stash_idx, &vreg_idx));
            struct kefir_codegen_amd64_xregalloc_virtual_register *vreg = &xregalloc->virtual_registers[vreg_idx];
            if (vreg->lifetime.begin == KEFIR_CODEGEN_AMD64_XREGALLOC_UNDEFINED) {
                vreg->lifetime.begin = linear_index;
                vreg->lifetime.end = linear_index;
            } else {
                vreg->lifetime.begin = MIN(vreg->lifetime.begin, linear_index);
                vreg->lifetime.end = MAX(vreg->lifetime.end, linear_index);
            }
            REQUIRE_OK(add_virtual_register_to_block(mem, &target->context, block_data, vreg_idx, vreg));
        } break;
    }
    return KEFIR_OK;
}

static kefir_result_t push_virtual_block(struct kefir_mem *mem, struct kefir_codegen_amd64_xregalloc *xregalloc,
                                         struct do_allocation_state *state,
                                         kefir_codegen_amd64_xregalloc_virtual_block_id_t virtual_block_id) {
    REQUIRE(!kefir_hashtree_has(&xregalloc->virtual_blocks, (kefir_hashtree_key_t) virtual_block_id),
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Duplicate virtual block identifier"));

    struct virtual_block_data *data = KEFIR_MALLOC(mem, sizeof(struct virtual_block_data));
    REQUIRE(data != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate virtual block data"));
    data->parent = state->current_virtual_block;
    data->virtual_block_id = virtual_block_id;
    kefir_result_t res = kefir_bucketset_init(&data->virtual_registers, &kefir_bucketset_uint_ops);
    REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &xregalloc->virtual_blocks, (kefir_hashtree_key_t) virtual_block_id,
                                              (kefir_hashtree_value_t) data));
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, data);
        return res;
    });
    state->current_virtual_block = data;
    if (state->current_virtual_block->parent == NULL) {
        state->base_virtual_block = state->current_virtual_block;
    }
    return KEFIR_OK;
}

static kefir_result_t pop_virtual_block(struct do_allocation_state *state) {
    REQUIRE(state->current_virtual_block != NULL && state->current_virtual_block->parent != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "No virtual block can be popped"));

    state->current_virtual_block = state->current_virtual_block->parent;
    return KEFIR_OK;
}

static kefir_result_t scan_code(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *code,
                                struct kefir_codegen_amd64_xregalloc *xregalloc, struct do_allocation_state *state) {
    kefir_size_t linear_index = 0;
    REQUIRE_OK(push_virtual_block(mem, xregalloc, state, KEFIR_CODEGEN_AMD64_XREGALLOC_VIRTUAL_BLOCK_DEFAULT_ID));
    for (kefir_asmcmp_instruction_index_t instr_index = kefir_asmcmp_context_instr_head(&code->context);
         instr_index != KEFIR_ASMCMP_INDEX_NONE;
         instr_index = kefir_asmcmp_context_instr_next(&code->context, instr_index), ++linear_index) {

        xregalloc->linearized_code[instr_index] = linear_index;

        struct kefir_asmcmp_instruction *instr;
        REQUIRE_OK(kefir_asmcmp_context_instr_at(&code->context, instr_index, &instr));

        switch (instr->opcode) {
            case KEFIR_ASMCMP_AMD64_OPCODE(virtual_block_begin):
                REQUIRE_OK(push_virtual_block(mem, xregalloc, state, instr->args[0].uint_immediate));
                break;

            case KEFIR_ASMCMP_AMD64_OPCODE(virtual_block_end):
                REQUIRE_OK(pop_virtual_block(state));
                break;

            case KEFIR_ASMCMP_AMD64_OPCODE(preserve_active_virtual_registers):
                REQUIRE_OK(
                    kefir_hashtree_insert(mem, &state->preserve_vreg_locations, (kefir_hashtree_key_t) linear_index,
                                          (kefir_hashtree_value_t) state->current_virtual_block->virtual_block_id));
                break;

            case KEFIR_ASMCMP_AMD64_OPCODE(stash_activate): {
                kefir_asmcmp_virtual_register_index_t vreg_idx;
                REQUIRE_OK(kefir_asmcmp_register_stash_vreg(&code->context, instr->args[0].stash_idx, &vreg_idx));
                REQUIRE_OK(kefir_hashtree_insert(mem, &state->stashes, (kefir_hashtree_key_t) vreg_idx,
                                                 (kefir_hashtree_value_t) instr_index));
            }
                // Fallthrough

            default:
                REQUIRE_OK(scan_virtual_register(mem, code, xregalloc, &instr->args[0], linear_index,
                                                 state->current_virtual_block));
                REQUIRE_OK(scan_virtual_register(mem, code, xregalloc, &instr->args[1], linear_index,
                                                 state->current_virtual_block));
                REQUIRE_OK(scan_virtual_register(mem, code, xregalloc, &instr->args[2], linear_index,
                                                 state->current_virtual_block));
                break;
        }
    }

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&state->preserve_vreg_locations, &iter);
         node != NULL; node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_size_t, linear_index, node->key);
        ASSIGN_DECL_CAST(kefir_codegen_amd64_xregalloc_virtual_block_id_t, block_id, node->value);

        struct kefir_codegen_amd64_xregalloc_virtual_block_iterator iter2;
        kefir_asmcmp_virtual_register_index_t vreg_idx;
        kefir_result_t res;
        for (res = kefir_codegen_amd64_xregalloc_block_iter(xregalloc, block_id, &iter2, &vreg_idx); res == KEFIR_OK;
             res = kefir_codegen_amd64_xregalloc_block_next(&iter2, &vreg_idx)) {
            struct kefir_codegen_amd64_xregalloc_virtual_register *vreg = &xregalloc->virtual_registers[vreg_idx];
            if (vreg->lifetime.begin <= linear_index && linear_index <= vreg->lifetime.end) {
                vreg->lifetime.begin = 0;
                vreg->lifetime.end = xregalloc->linear_code_length;
                REQUIRE_OK(
                    add_virtual_register_to_block(mem, &code->context, state->base_virtual_block, vreg_idx, vreg));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t mark_virtual_register_interference(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *code,
                                                         struct kefir_codegen_amd64_xregalloc *xregalloc,
                                                         const struct kefir_asmcmp_virtual_register *asmcmp_vreg1,
                                                         const struct kefir_asmcmp_virtual_register *asmcmp_vreg2) {
    struct kefir_codegen_amd64_xregalloc_virtual_register *vreg1 = &xregalloc->virtual_registers[asmcmp_vreg1->index];
    struct kefir_codegen_amd64_xregalloc_virtual_register *vreg2 = &xregalloc->virtual_registers[asmcmp_vreg2->index];
    if (asmcmp_vreg1->index == asmcmp_vreg2->index) {
        return KEFIR_OK;
    }
    if (asmcmp_vreg1->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_LOCAL_VARIABLE ||
        asmcmp_vreg1->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_INTEGER) {
        return KEFIR_OK;
    }
    if (asmcmp_vreg2->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_LOCAL_VARIABLE ||
        asmcmp_vreg2->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_INTEGER) {
        return KEFIR_OK;
    }

    if (asmcmp_vreg1->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR) {
        const struct kefir_asmcmp_virtual_register *first_vreg, *second_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&code->context, asmcmp_vreg1->parameters.pair.virtual_registers[0],
                                                     &first_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&code->context, asmcmp_vreg1->parameters.pair.virtual_registers[1],
                                                     &second_vreg));
        REQUIRE_OK(mark_virtual_register_interference(mem, code, xregalloc, first_vreg, asmcmp_vreg2));
        REQUIRE_OK(mark_virtual_register_interference(mem, code, xregalloc, second_vreg, asmcmp_vreg2));
    }
    if (asmcmp_vreg2->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR) {
        const struct kefir_asmcmp_virtual_register *first_vreg, *second_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&code->context, asmcmp_vreg2->parameters.pair.virtual_registers[0],
                                                     &first_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&code->context, asmcmp_vreg2->parameters.pair.virtual_registers[1],
                                                     &second_vreg));
        REQUIRE_OK(mark_virtual_register_interference(mem, code, xregalloc, asmcmp_vreg1, first_vreg));
        REQUIRE_OK(mark_virtual_register_interference(mem, code, xregalloc, asmcmp_vreg1, second_vreg));
    }

    if (vreg1->lifetime.begin < vreg2->lifetime.end && vreg2->lifetime.begin < vreg1->lifetime.end) {
        REQUIRE_OK(kefir_bucketset_add(mem, &vreg1->interference, (kefir_bucketset_entry_t) asmcmp_vreg2->index));
        REQUIRE_OK(kefir_bucketset_add(mem, &vreg2->interference, (kefir_bucketset_entry_t) asmcmp_vreg1->index));
    }
    return KEFIR_OK;
}

static kefir_result_t build_virtual_register_interference(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *code,
                                                          struct kefir_codegen_amd64_xregalloc *xregalloc) {
    struct kefir_hashtree_node_iterator iter;
    for (struct kefir_hashtree_node *node = kefir_hashtree_iter(&xregalloc->virtual_blocks, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_codegen_amd64_xregalloc_virtual_block_id_t, virtual_block_id, node->key);

        struct kefir_codegen_amd64_xregalloc_virtual_block_iterator vreg_iter1, vreg_iter2;
        const struct kefir_asmcmp_virtual_register *asmcmp_vreg1, *asmcmp_vreg2;
        kefir_asmcmp_virtual_register_index_t vreg1_idx, vreg2_idx;
        kefir_result_t res;
        for (res = kefir_codegen_amd64_xregalloc_block_iter(xregalloc, virtual_block_id, &vreg_iter1, &vreg1_idx);
             res == KEFIR_OK; res = kefir_codegen_amd64_xregalloc_block_next(&vreg_iter1, &vreg1_idx)) {
            REQUIRE_OK(kefir_asmcmp_virtual_register_get(&code->context, vreg1_idx, &asmcmp_vreg1));
            if (asmcmp_vreg1->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_LOCAL_VARIABLE ||
                asmcmp_vreg1->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_INTEGER) {
                continue;
            }
            for (res = kefir_codegen_amd64_xregalloc_block_iter(xregalloc, virtual_block_id, &vreg_iter2, &vreg2_idx);
                 res == KEFIR_OK; res = kefir_codegen_amd64_xregalloc_block_next(&vreg_iter2, &vreg2_idx)) {
                if (vreg1_idx == vreg2_idx) {
                    continue;
                }
                REQUIRE_OK(kefir_asmcmp_virtual_register_get(&code->context, vreg2_idx, &asmcmp_vreg2));
                if (asmcmp_vreg2->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_LOCAL_VARIABLE ||
                    asmcmp_vreg2->type == KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_INTEGER) {
                    continue;
                }

                REQUIRE_OK(mark_virtual_register_interference(mem, code, xregalloc, asmcmp_vreg1, asmcmp_vreg2));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t build_virtual_register_allocation_order(struct kefir_mem *mem,
                                                              struct kefir_codegen_amd64_xregalloc *xregalloc,
                                                              struct do_allocation_state *state) {
    for (kefir_size_t i = 0; i < xregalloc->virtual_register_length; i++) {
        struct kefir_codegen_amd64_xregalloc_virtual_register *vreg = &xregalloc->virtual_registers[i];
        if (vreg->lifetime.begin == KEFIR_CODEGEN_AMD64_XREGALLOC_UNDEFINED) {
            continue;
        }

        // Prioritize virtual registers with shorter lifetimes to avoid clobbering
        // available physical registers with long-living values.
        // Group virtual registers by lifetime (5 bits are rounded down).
        // Inside these groups, prioritize virtual registers appearing earlier to
        // take into account allocation hints.
        // Virtual registers whose lifetime encompasses the whole function,
        // are deprioritized.
        const kefir_bool_t whole_function_lifetime =
            vreg->lifetime.begin == 0 && vreg->lifetime.end == xregalloc->linear_code_length &&
            kefir_bucketset_has(&state->base_virtual_block->virtual_registers, (kefir_bucketset_entry_t) i);
        const kefir_uint64_t lifetime_duration = vreg->lifetime.end - vreg->lifetime.begin;
        const kefir_uint64_t lifetime_round_bits = 5;
        const kefir_uint64_t mask = (1ull << 32) - 1;
        const kefir_uint32_t upper_half =
            whole_function_lifetime ? mask : (lifetime_duration >> lifetime_round_bits) & mask;
        const kefir_uint32_t lower_half = vreg->lifetime.begin & mask;
        const kefir_size_t priority = (((kefir_uint64_t) upper_half) << 32) | lower_half;

        struct virtual_register_allocation_order_entry *entry;
        struct kefir_hashtree_node *node;
        kefir_result_t res = kefir_hashtree_at(&state->allocation_order, (kefir_hashtree_key_t) priority, &node);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            entry = (struct virtual_register_allocation_order_entry *) node->value;
        } else {
            entry = KEFIR_MALLOC(mem, sizeof(struct virtual_register_allocation_order_entry));
            REQUIRE(entry != NULL,
                    KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 register allocator order entry"));
            res = kefir_hashtreeset_init(&entry->virtual_registers, &kefir_hashtree_uint_ops);
            REQUIRE_CHAIN(&res, kefir_hashtree_insert(mem, &state->allocation_order, (kefir_hashtree_key_t) priority,
                                                      (kefir_hashtree_value_t) entry));
            REQUIRE_ELSE(res == KEFIR_OK, {
                KEFIR_FREE(mem, entry);
                return res;
            });
        }
        REQUIRE_OK(kefir_hashtreeset_add(mem, &entry->virtual_registers, (kefir_hashtreeset_entry_t) i));
    }
    return KEFIR_OK;
}

static kefir_result_t add_active_virtual_register(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *code,
                                                  struct kefir_codegen_amd64_xregalloc *xregalloc,
                                                  struct do_allocation_state *state,
                                                  struct kefir_codegen_amd64_xregalloc_virtual_register *vreg,
                                                  kefir_asmcmp_virtual_register_index_t interfere_vreg_idx) {
    const struct kefir_codegen_amd64_xregalloc_virtual_register *interfere_vreg =
        &xregalloc->virtual_registers[interfere_vreg_idx];
    const struct kefir_asmcmp_virtual_register *asmcmp_interfere_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&code->context, interfere_vreg_idx, &asmcmp_interfere_vreg));

    switch (interfere_vreg->allocation.type) {
        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED: {
            const struct kefir_asmcmp_amd64_register_preallocation *preallocation;
            REQUIRE_OK(kefir_asmcmp_amd64_get_register_preallocation(code, interfere_vreg_idx, &preallocation));
            if (preallocation != NULL) {
                switch (preallocation->type) {
                    case KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT: {
                        ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, preallocation->reg);
                        REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
                        REQUIRE_OK(
                            kefir_hashtreeset_add(mem, &state->active_registers, (kefir_hashtreeset_entry_t) reg));
                    } break;

                    case KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_HINT: {
                        ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, preallocation->reg);
                        REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
                        REQUIRE_OK(kefir_hashtreeset_add(mem, &state->active_hints, (kefir_hashtreeset_entry_t) reg));
                    } break;

                    case KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_SAME_AS: {
                        const struct kefir_codegen_amd64_xregalloc_virtual_register *other_vreg =
                            &xregalloc->virtual_registers[preallocation->vreg];
                        if (other_vreg->allocation.type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER) {
                            REQUIRE_OK(
                                kefir_hashtreeset_add(mem, &state->active_hints,
                                                      (kefir_hashtreeset_entry_t) other_vreg->allocation.direct_reg));
                        } else if (other_vreg->allocation.type ==
                                       KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT ||
                                   other_vreg->allocation.type ==
                                       KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_INDIRECT) {
                            REQUIRE_OK(kefir_bitset_ensure(
                                mem, &state->active_spill_area_hints,
                                other_vreg->allocation.spill_area.index + other_vreg->allocation.spill_area.length));
                            REQUIRE_OK(kefir_bitset_set_consecutive(&state->active_spill_area_hints,
                                                                    other_vreg->allocation.spill_area.index,
                                                                    other_vreg->allocation.spill_area.length, true));
                        }
                    } break;
                }
            }
        } break;

        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER:
            REQUIRE_OK(kefir_hashtreeset_add(mem, &state->active_registers,
                                             (kefir_hashtreeset_entry_t) interfere_vreg->allocation.direct_reg));
            break;

        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT:
        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_INDIRECT: {
            REQUIRE_OK(kefir_bitset_ensure(
                mem, &state->active_spill_area,
                interfere_vreg->allocation.spill_area.index + interfere_vreg->allocation.spill_area.length));
            REQUIRE_OK(kefir_bitset_set_consecutive(&state->active_spill_area,
                                                    interfere_vreg->allocation.spill_area.index,
                                                    interfere_vreg->allocation.spill_area.length, true));
        } break;

        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_PAIR:
            REQUIRE_OK(add_active_virtual_register(mem, code, xregalloc, state, vreg,
                                                   asmcmp_interfere_vreg->parameters.pair.virtual_registers[0]));
            REQUIRE_OK(add_active_virtual_register(mem, code, xregalloc, state, vreg,
                                                   asmcmp_interfere_vreg->parameters.pair.virtual_registers[1]));
            break;

        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_IMMEDIATE_INTEGER:
        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_LOCAL_VARIABLE:
        case KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER:
            // Intentionally left blank
            break;
    }

    return KEFIR_OK;
}

static kefir_result_t build_active_virtual_registers(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *code,
                                                     struct kefir_codegen_amd64_xregalloc *xregalloc,
                                                     struct do_allocation_state *state,
                                                     struct kefir_codegen_amd64_xregalloc_virtual_register *vreg) {
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &state->active_registers));
    REQUIRE_OK(kefir_hashtreeset_clean(mem, &state->active_hints));
    REQUIRE_OK(kefir_bitset_clear(&state->active_spill_area));
    REQUIRE_OK(kefir_bitset_clear(&state->active_spill_area_hints));

    struct kefir_bucketset_iterator iter;
    kefir_bucketset_entry_t entry;
    kefir_result_t res;
    for (res = kefir_bucketset_iter(&vreg->interference, &iter, &entry); res == KEFIR_OK;
         res = kefir_bucketset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, interfere_vreg_idx, entry);
        REQUIRE_OK(add_active_virtual_register(mem, code, xregalloc, state, vreg, interfere_vreg_idx));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t allocate_spill_area(struct kefir_mem *mem, struct do_allocation_state *state,
                                          struct kefir_codegen_amd64_xregalloc_virtual_register *vreg,
                                          kefir_size_t length, kefir_size_t alignment) {
    if (length == 0) {
        vreg->allocation.spill_area.index = 0;
        vreg->allocation.spill_area.length = 0;
        return KEFIR_OK;
    }

    kefir_size_t spill_index;
    kefir_size_t num_of_slots;
    REQUIRE_OK(kefir_bitset_length(&state->active_spill_area, &num_of_slots));

    kefir_size_t iter_index;
    for (iter_index = 0; iter_index < num_of_slots;) {
        kefir_result_t res =
            kefir_bitset_find_consecutive(&state->active_spill_area, false, length, iter_index, &spill_index);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            kefir_bool_t satisfies_reqs = spill_index % alignment == 0;
            for (kefir_size_t i = spill_index; satisfies_reqs && i < spill_index + length; i++) {
                kefir_bool_t hint_occupied;
                REQUIRE_OK(kefir_bitset_get(&state->active_spill_area_hints, i, &hint_occupied));
                satisfies_reqs = satisfies_reqs && !hint_occupied;
            }
            if (satisfies_reqs) {
                vreg->allocation.spill_area.index = spill_index;
                vreg->allocation.spill_area.length = length;
                return KEFIR_OK;
            } else {
                iter_index = spill_index + 1;
            }
        } else {
            iter_index = num_of_slots;
        }
    }

    const kefir_size_t orig_num_of_slots = num_of_slots;
    num_of_slots = kefir_target_abi_pad_aligned(num_of_slots + length, alignment);
    REQUIRE_OK(kefir_bitset_resize(mem, &state->active_spill_area, num_of_slots));
    REQUIRE_OK(kefir_bitset_resize(mem, &state->active_spill_area_hints, num_of_slots));

    for (iter_index = orig_num_of_slots; iter_index < num_of_slots;) {
        kefir_result_t res =
            kefir_bitset_find_consecutive(&state->active_spill_area, false, length, iter_index, &spill_index);
        if (res != KEFIR_NOT_FOUND) {
            REQUIRE_OK(res);
            if (spill_index % alignment == 0) {
                vreg->allocation.spill_area.index = spill_index;
                vreg->allocation.spill_area.length = length;
                return KEFIR_OK;
            } else {
                iter_index = spill_index + 1;
            }
        } else {
            iter_index = num_of_slots;
        }
    }

    return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unable to allocate spill space");
}

static kefir_result_t do_allocate_register(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *code,
                                           struct kefir_codegen_amd64_xregalloc *xregalloc,
                                           struct do_allocation_state *state,
                                           kefir_asmcmp_virtual_register_index_t vreg_idx,
                                           struct kefir_codegen_amd64_xregalloc_virtual_register *vreg,
                                           const kefir_asm_amd64_xasmgen_register_t *available_registers,
                                           kefir_size_t available_registers_length, kefir_size_t spill_area_size) {
    const struct kefir_asmcmp_amd64_register_preallocation *preallocation;
    REQUIRE_OK(kefir_asmcmp_amd64_get_register_preallocation(code, vreg_idx, &preallocation));

    if (preallocation != NULL && preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_REQUIREMENT) {
        ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, preallocation->reg);
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
        REQUIRE(!kefir_hashtreeset_has(&state->active_registers, (kefir_hashtreeset_entry_t) reg),
                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to satisfy register allocation requirements"));
        vreg->allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER;
        vreg->allocation.direct_reg = reg;
        REQUIRE_OK(kefir_hashtreeset_add(mem, &state->active_registers, (kefir_hashtreeset_entry_t) reg));
        return KEFIR_OK;
    }

    if (preallocation != NULL && preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_HINT) {
        ASSIGN_DECL_CAST(kefir_asm_amd64_xasmgen_register_t, reg, preallocation->reg);
        REQUIRE_OK(kefir_asm_amd64_xasmgen_register_widest(reg, &reg));
        if (!kefir_hashtreeset_has(&state->active_registers, (kefir_hashtreeset_entry_t) reg)) {
            for (kefir_size_t i = 0; i < available_registers_length; i++) {
                const kefir_asm_amd64_xasmgen_register_t available_reg = available_registers[i];
                if (available_reg == reg) {
                    vreg->allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER;
                    vreg->allocation.direct_reg = reg;
                    REQUIRE_OK(kefir_hashtreeset_add(mem, &state->active_registers, (kefir_hashtreeset_entry_t) reg));
                    return KEFIR_OK;
                }
            }
        }
    }

    if (preallocation != NULL && preallocation->type == KEFIR_ASMCMP_AMD64_REGISTER_PREALLOCATION_SAME_AS &&
        !kefir_bucketset_has(&vreg->interference, preallocation->vreg)) {
        const struct kefir_codegen_amd64_xregalloc_virtual_register *other_vreg =
            &xregalloc->virtual_registers[preallocation->vreg];

        const struct kefir_asmcmp_virtual_register *asmcmp_vreg, *asmcmp_other_vreg;
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&code->context, vreg_idx, &asmcmp_vreg));
        REQUIRE_OK(kefir_asmcmp_virtual_register_get(&code->context, preallocation->vreg, &asmcmp_other_vreg));
        if (asmcmp_vreg->type == asmcmp_other_vreg->type) {
            if (other_vreg->allocation.type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER) {
                const kefir_asm_amd64_xasmgen_register_t reg = other_vreg->allocation.direct_reg;
                if (!kefir_hashtreeset_has(&state->active_registers, (kefir_hashtreeset_entry_t) reg)) {
                    for (kefir_size_t i = 0; i < available_registers_length; i++) {
                        const kefir_asm_amd64_xasmgen_register_t available_reg = available_registers[i];
                        if (available_reg == reg) {
                            vreg->allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER;
                            vreg->allocation.direct_reg = reg;
                            REQUIRE_OK(
                                kefir_hashtreeset_add(mem, &state->active_registers, (kefir_hashtreeset_entry_t) reg));
                            return KEFIR_OK;
                        }
                    }
                }
            } else if (other_vreg->allocation.type ==
                       KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT) {
                kefir_bool_t occupied = false;
                for (kefir_size_t i = 0; !occupied && i < other_vreg->allocation.spill_area.length; i++) {
                    kefir_bool_t slot_occupied;
                    REQUIRE_OK(kefir_bitset_get(&state->active_spill_area, i + other_vreg->allocation.spill_area.index,
                                                &slot_occupied));
                    occupied = occupied || slot_occupied;
                }
                if (!occupied) {
                    REQUIRE_OK(kefir_bitset_set_consecutive(&state->active_spill_area,
                                                            other_vreg->allocation.spill_area.index,
                                                            other_vreg->allocation.spill_area.length, true));
                    vreg->allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT;
                    vreg->allocation.spill_area.index = other_vreg->allocation.spill_area.index;
                    vreg->allocation.spill_area.length = other_vreg->allocation.spill_area.length;
                    return KEFIR_OK;
                }
            }
        }
    }

#define ALLOC_REG(_reg)                                                                                       \
    do {                                                                                                      \
        vreg->allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER;                     \
        vreg->allocation.direct_reg = (_reg);                                                                 \
        REQUIRE_OK(kefir_hashtreeset_add(mem, &state->active_registers, (kefir_hashtreeset_entry_t) (_reg))); \
        return KEFIR_OK;                                                                                      \
    } while (0)
    for (kefir_size_t i = 0; i < available_registers_length; i++) {
        const kefir_asm_amd64_xasmgen_register_t reg = available_registers[i];
        if (!kefir_hashtreeset_has(&state->active_registers, (kefir_hashtreeset_entry_t) reg) &&
            !kefir_hashtreeset_has(&state->active_hints, (kefir_hashtreeset_entry_t) reg)) {
            ALLOC_REG(reg);
        }
    }

    for (kefir_size_t i = 0; i < available_registers_length; i++) {
        const kefir_asm_amd64_xasmgen_register_t reg = available_registers[i];
        if (!kefir_hashtreeset_has(&state->active_registers, (kefir_hashtreeset_entry_t) reg)) {
            ALLOC_REG(reg);
        }
    }
#undef ALLOC_REG

    REQUIRE_OK(allocate_spill_area(mem, state, vreg, spill_area_size, spill_area_size));
    vreg->allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_DIRECT;

    return KEFIR_OK;
}

static kefir_result_t do_vreg_allocation(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *code,
                                         struct kefir_codegen_amd64_stack_frame *stack_frame,
                                         struct kefir_codegen_amd64_xregalloc *xregalloc,
                                         struct do_allocation_state *state,
                                         kefir_asmcmp_virtual_register_index_t vreg_idx) {
    UNUSED(stack_frame);
    struct kefir_codegen_amd64_xregalloc_virtual_register *vreg = &xregalloc->virtual_registers[vreg_idx];
    REQUIRE(vreg->allocation.type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED, KEFIR_OK);

    const struct kefir_asmcmp_virtual_register *asmcmp_vreg;
    REQUIRE_OK(kefir_asmcmp_virtual_register_get(&code->context, vreg_idx, &asmcmp_vreg));

    switch (asmcmp_vreg->type) {
        case KEFIR_ASMCMP_VIRTUAL_REGISTER_UNSPECIFIED:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected amd64 virtual register type");

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_GENERAL_PURPOSE:
            REQUIRE_OK(build_active_virtual_registers(mem, code, xregalloc, state, vreg));
            REQUIRE_OK(do_allocate_register(mem, code, xregalloc, state, vreg_idx, vreg,
                                            xregalloc->available_registers.general_purpose_registers,
                                            xregalloc->available_registers.num_of_general_purpose_registers, 1));
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_FLOATING_POINT:
            REQUIRE_OK(build_active_virtual_registers(mem, code, xregalloc, state, vreg));
            REQUIRE_OK(do_allocate_register(mem, code, xregalloc, state, vreg_idx, vreg,
                                            xregalloc->available_registers.floating_point_registers,
                                            xregalloc->available_registers.num_of_floating_point_registers, 2));
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_SPILL_SPACE:
            REQUIRE_OK(build_active_virtual_registers(mem, code, xregalloc, state, vreg));
            REQUIRE_OK(allocate_spill_area(mem, state, vreg, asmcmp_vreg->parameters.spill_space_allocation.length,
                                           asmcmp_vreg->parameters.spill_space_allocation.alignment));
            vreg->allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_SPILL_AREA_INDIRECT;
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_LOCAL_VARIABLE:
            vreg->allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_LOCAL_VARIABLE;
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_IMMEDIATE_INTEGER:
            vreg->allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_IMMEDIATE_INTEGER;
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_EXTERNAL_MEMORY:
            vreg->allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_MEMORY_POINTER;
            break;

        case KEFIR_ASMCMP_VIRTUAL_REGISTER_PAIR:
            REQUIRE_OK(do_vreg_allocation(mem, code, stack_frame, xregalloc, state,
                                          asmcmp_vreg->parameters.pair.virtual_registers[0]));
            REQUIRE_OK(do_vreg_allocation(mem, code, stack_frame, xregalloc, state,
                                          asmcmp_vreg->parameters.pair.virtual_registers[1]));
            vreg->allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_PAIR;
            break;
    }
    REQUIRE_OK(kefir_hashtreeset_merge(mem, &xregalloc->used_registers, &state->active_registers, NULL, NULL));

    return KEFIR_OK;
}

static kefir_result_t do_stash_allocation(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *code,
                                          struct kefir_codegen_amd64_stack_frame *stack_frame,
                                          struct kefir_codegen_amd64_xregalloc *xregalloc,
                                          struct do_allocation_state *state,
                                          kefir_asmcmp_virtual_register_index_t vreg_idx,
                                          kefir_asmcmp_instruction_index_t activate_instr_idx) {
    struct kefir_codegen_amd64_xregalloc_virtual_register *vreg = &xregalloc->virtual_registers[vreg_idx];
    REQUIRE(vreg->allocation.type == KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED, KEFIR_OK);

    struct kefir_asmcmp_instruction *activate_instr;
    REQUIRE_OK(kefir_asmcmp_context_instr_at(&code->context, activate_instr_idx, &activate_instr));
    const kefir_asmcmp_stash_index_t stash_idx = activate_instr->args[0].stash_idx;

    kefir_asmcmp_instruction_index_t liveness_instr_idx;
    REQUIRE_OK(kefir_asmcmp_register_stash_liveness_index(&code->context, stash_idx, &liveness_instr_idx));
    if (liveness_instr_idx == KEFIR_ASMCMP_INDEX_NONE) {
        liveness_instr_idx = activate_instr_idx;
    }
    const kefir_size_t activation_linear_idx = xregalloc->linearized_code[activate_instr_idx];
    const kefir_size_t liveness_linear_idx = xregalloc->linearized_code[liveness_instr_idx];

    REQUIRE_OK(build_active_virtual_registers(mem, code, xregalloc, state, vreg));
    kefir_result_t res;
    struct kefir_bucketset_iterator iter;
    kefir_bucketset_entry_t entry;
    kefir_size_t qwords = 0;
    for (res = kefir_bucketset_iter(&vreg->interference, &iter, &entry); res == KEFIR_OK;
         res = kefir_bucketset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, interfere_vreg_idx, entry);
        const struct kefir_codegen_amd64_xregalloc_virtual_register *interfere_vreg =
            &xregalloc->virtual_registers[interfere_vreg_idx];
        if (interfere_vreg->allocation.type != KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_ALLOCATION_REGISTER) {
            continue;
        }

        kefir_bool_t contains_phreg, contains_vreg;
        kefir_result_t res = kefir_asmcmp_register_stash_has(&code->context, stash_idx,
                                                             interfere_vreg->allocation.direct_reg, &contains_phreg);
        REQUIRE_CHAIN(&res, kefir_asmcmp_register_stash_has_virtual(&code->context, stash_idx, interfere_vreg_idx,
                                                                    &contains_vreg));
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        if (!contains_phreg || !contains_vreg) {
            continue;
        }

        if (interfere_vreg->lifetime.begin > liveness_linear_idx ||
            interfere_vreg->lifetime.end <= liveness_linear_idx ||
            interfere_vreg->lifetime.begin > activation_linear_idx ||
            interfere_vreg->lifetime.end <= activation_linear_idx) {
            continue;
        }

        if (!kefir_asm_amd64_xasmgen_register_is_floating_point(interfere_vreg->allocation.direct_reg)) {
            qwords++;
        } else {
            qwords += 2;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_asmcmp_virtual_register_index_t spill_vreg;
    REQUIRE_OK(kefir_asmcmp_register_stash_vreg(&code->context, stash_idx, &spill_vreg));
    REQUIRE_OK(kefir_asmcmp_virtual_set_spill_space_size(&code->context, spill_vreg, qwords, 1));
    REQUIRE_OK(do_vreg_allocation(mem, code, stack_frame, xregalloc, state, vreg_idx));
    return KEFIR_OK;
}

static kefir_result_t do_allocation_impl(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *code,
                                         struct kefir_codegen_amd64_stack_frame *stack_frame,
                                         struct kefir_codegen_amd64_xregalloc *xregalloc,
                                         struct do_allocation_state *state) {
    REQUIRE_OK(build_virtual_register_allocation_order(mem, xregalloc, state));

    struct kefir_hashtree_node *node;
    REQUIRE_OK(kefir_hashtree_min(&state->allocation_order, &node));
    for (; node != NULL; node = kefir_hashtree_next_node(&state->allocation_order, node)) {
        ASSIGN_DECL_CAST(struct virtual_register_allocation_order_entry *, entry, node->value);

        kefir_result_t res;
        struct kefir_hashtreeset_iterator iter;
        for (res = kefir_hashtreeset_iter(&entry->virtual_registers, &iter); res == KEFIR_OK;
             res = kefir_hashtreeset_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, iter.entry);
            if (!kefir_hashtree_has(&state->stashes, (kefir_hashtree_key_t) vreg_idx)) {
                REQUIRE_OK(do_vreg_allocation(mem, code, stack_frame, xregalloc, state, vreg_idx));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    struct kefir_hashtree_node_iterator iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&state->stashes, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_asmcmp_virtual_register_index_t, vreg_idx, node->key);
        ASSIGN_DECL_CAST(kefir_asmcmp_instruction_index_t, instr_idx, node->value);
        REQUIRE_OK(do_stash_allocation(mem, code, stack_frame, xregalloc, state, vreg_idx, instr_idx));
    }

    kefir_size_t num_of_slots;
    REQUIRE_OK(kefir_bitset_length(&state->active_spill_area, &num_of_slots));
    REQUIRE_OK(kefir_codegen_amd64_stack_frame_ensure_spill_area(stack_frame, num_of_slots));
    xregalloc->used_slots = MAX(xregalloc->used_slots, num_of_slots);

    kefir_result_t res;
    struct kefir_hashtreeset_iterator used_reg_iter;
    for (res = kefir_hashtreeset_iter(&xregalloc->used_registers, &used_reg_iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&used_reg_iter)) {
        REQUIRE_OK(kefir_codegen_amd64_stack_frame_use_register(
            mem, stack_frame, (kefir_asm_amd64_xasmgen_register_t) used_reg_iter.entry));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t virtual_register_allocation_order_entry_free(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                                   kefir_hashtree_key_t key,
                                                                   kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct virtual_register_allocation_order_entry *, entry, value);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Expected valid register allocator lifetime entry"));

    REQUIRE_OK(kefir_hashtreeset_free(mem, &entry->virtual_registers));
    KEFIR_FREE(mem, entry);
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

kefir_result_t kefir_codegen_amd64_xregalloc_run(struct kefir_mem *mem, struct kefir_asmcmp_amd64 *code,
                                                 struct kefir_codegen_amd64_stack_frame *stack_frame,
                                                 struct kefir_codegen_amd64_xregalloc *xregalloc) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(code != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 code"));
    REQUIRE(stack_frame != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 stack frame"));
    REQUIRE(xregalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));

    REQUIRE(xregalloc->linearized_code == NULL && xregalloc->virtual_registers == NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Register allocator has already been executed"));
    xregalloc->linear_code_length = code->context.code_length;
    xregalloc->virtual_register_length = code->context.virtual_register_length;
    xregalloc->linearized_code = KEFIR_MALLOC(mem, sizeof(kefir_size_t) * xregalloc->linear_code_length);
    xregalloc->virtual_registers = KEFIR_MALLOC(
        mem, sizeof(struct kefir_codegen_amd64_xregalloc_virtual_register) * xregalloc->virtual_register_length);
    REQUIRE(xregalloc->linearized_code != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 register allocator state"));
    REQUIRE(xregalloc->virtual_registers != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 register allocator state"));
    for (kefir_size_t i = 0; i < xregalloc->virtual_register_length; i++) {
        xregalloc->virtual_registers[i].allocation.type = KEFIR_CODEGEN_AMD64_VIRTUAL_REGISTER_UNALLOCATED;
        xregalloc->virtual_registers[i].lifetime.begin = KEFIR_CODEGEN_AMD64_XREGALLOC_UNDEFINED;
        xregalloc->virtual_registers[i].lifetime.end = KEFIR_CODEGEN_AMD64_XREGALLOC_UNDEFINED;
        REQUIRE_OK(kefir_bucketset_init(&xregalloc->virtual_registers[i].interference, &kefir_bucketset_uint_ops));
        REQUIRE_OK(kefir_bucketset_init(&xregalloc->virtual_registers[i].virtual_blocks, &kefir_bucketset_uint_ops));
    }

    xregalloc->available_registers.num_of_general_purpose_registers = NUM_OF_AMD64_GENERAL_PURPOSE_REGS;
    xregalloc->available_registers.num_of_floating_point_registers = NUM_OF_AMD64_FLOATING_POINT_REGS;
    xregalloc->available_registers.general_purpose_registers =
        KEFIR_MALLOC(mem, sizeof(kefir_asm_amd64_xasmgen_register_t) *
                              xregalloc->available_registers.num_of_general_purpose_registers);
    xregalloc->available_registers.floating_point_registers =
        KEFIR_MALLOC(mem, sizeof(kefir_asm_amd64_xasmgen_register_t) *
                              xregalloc->available_registers.num_of_floating_point_registers);
    REQUIRE(xregalloc->available_registers.general_purpose_registers != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 register allocator state"));
    REQUIRE(xregalloc->available_registers.floating_point_registers != NULL,
            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate amd64 register allocator state"));
    memcpy(
        xregalloc->available_registers.general_purpose_registers, AMD64_GENERAL_PURPOSE_REGS,
        sizeof(kefir_asm_amd64_xasmgen_register_t) * xregalloc->available_registers.num_of_general_purpose_registers);
    memcpy(xregalloc->available_registers.floating_point_registers, AMD64_FLOATING_POINT_REGS,
           sizeof(kefir_asm_amd64_xasmgen_register_t) * xregalloc->available_registers.num_of_floating_point_registers);

    REQUIRE_OK(kefir_mergesort(
        mem, xregalloc->available_registers.general_purpose_registers, sizeof(kefir_asm_amd64_xasmgen_register_t),
        xregalloc->available_registers.num_of_general_purpose_registers, abi_register_comparator, code));
    REQUIRE_OK(kefir_mergesort(
        mem, xregalloc->available_registers.floating_point_registers, sizeof(kefir_asm_amd64_xasmgen_register_t),
        xregalloc->available_registers.num_of_floating_point_registers, abi_register_comparator, code));

    struct do_allocation_state state = {.current_virtual_block = NULL};
    REQUIRE_OK(kefir_hashtree_init(&state.allocation_order, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&state.allocation_order, virtual_register_allocation_order_entry_free, NULL));
    REQUIRE_OK(kefir_hashtree_init(&state.stashes, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&state.preserve_vreg_locations, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.active_registers, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.active_hints, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_bitset_init(&state.active_spill_area));
    REQUIRE_OK(kefir_bitset_init(&state.active_spill_area_hints));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, scan_code(mem, code, xregalloc, &state));
    REQUIRE_CHAIN(&res, build_virtual_register_interference(mem, code, xregalloc));
    REQUIRE_CHAIN(&res, do_allocation_impl(mem, code, stack_frame, xregalloc, &state));

    kefir_hashtree_free(mem, &state.allocation_order);
    kefir_hashtree_free(mem, &state.stashes);
    kefir_hashtree_free(mem, &state.preserve_vreg_locations);
    kefir_hashtreeset_free(mem, &state.active_registers);
    kefir_hashtreeset_free(mem, &state.active_hints);
    kefir_bitset_free(mem, &state.active_spill_area);
    kefir_bitset_free(mem, &state.active_spill_area_hints);
    return res;
}

kefir_result_t kefir_codegen_amd64_xregalloc_allocation_of(
    const struct kefir_codegen_amd64_xregalloc *xregalloc, kefir_asmcmp_virtual_register_index_t vreg_idx,
    const struct kefir_codegen_amd64_register_allocation **vreg_ptr) {
    REQUIRE(xregalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(vreg_idx < xregalloc->virtual_register_length,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find register allocation"));
    REQUIRE(vreg_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 pointer to register allocation"));

    *vreg_ptr = &xregalloc->virtual_registers[vreg_idx].allocation;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_xregalloc_linear_position_of(const struct kefir_codegen_amd64_xregalloc *xregalloc,
                                                                kefir_asmcmp_instruction_index_t instr_idx,
                                                                kefir_size_t *linear_position_ptr) {
    REQUIRE(xregalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(instr_idx < xregalloc->linear_code_length,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find instruction linear position"));
    REQUIRE(linear_position_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 pointer to linear position"));

    *linear_position_ptr = xregalloc->linearized_code[instr_idx];
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_xregalloc_lifetime_of(const struct kefir_codegen_amd64_xregalloc *xregalloc,
                                                         kefir_asmcmp_virtual_register_index_t vreg_idx,
                                                         kefir_size_t *begin_ptr, kefir_size_t *end_ptr) {
    REQUIRE(xregalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(vreg_idx < xregalloc->virtual_register_length,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find virtual register"));

    ASSIGN_PTR(begin_ptr, xregalloc->virtual_registers[vreg_idx].lifetime.begin);
    ASSIGN_PTR(end_ptr, xregalloc->virtual_registers[vreg_idx].lifetime.end);
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_amd64_xregalloc_exists_in_block(const struct kefir_codegen_amd64_xregalloc *xregalloc,
                                                             kefir_asmcmp_virtual_register_index_t vreg_idx,
                                                             kefir_codegen_amd64_xregalloc_virtual_block_id_t block_id,
                                                             kefir_bool_t *exists_flag) {
    REQUIRE(xregalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(vreg_idx < xregalloc->virtual_register_length,
            KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find virtual register"));
    REQUIRE(exists_flag != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&xregalloc->virtual_blocks, (kefir_hashtree_key_t) block_id, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find virtual block");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct virtual_block_data *, virtual_block, node->value);
    kefir_bool_t exists = false;
    for (; !exists && virtual_block != NULL; virtual_block = virtual_block->parent) {
        if (kefir_bucketset_has(&virtual_block->virtual_registers, (kefir_bucketset_entry_t) vreg_idx)) {
            exists = true;
        }
    }
    *exists_flag = exists;
    return KEFIR_OK;
}

kefir_bool_t kefir_codegen_amd64_xregalloc_has_used_register(const struct kefir_codegen_amd64_xregalloc *xregalloc,
                                                             kefir_asm_amd64_xasmgen_register_t reg) {
    REQUIRE(xregalloc != NULL, false);
    return kefir_hashtreeset_has(&xregalloc->used_registers, reg);
}

kefir_result_t kefir_codegen_amd64_xregalloc_block_iter(
    const struct kefir_codegen_amd64_xregalloc *xregalloc, kefir_codegen_amd64_xregalloc_virtual_block_id_t block_id,
    struct kefir_codegen_amd64_xregalloc_virtual_block_iterator *iter,
    kefir_asmcmp_virtual_register_index_t *vreg_idx_ptr) {
    REQUIRE(xregalloc != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator"));
    REQUIRE(iter != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER,
                                          "Expected valid pointer to amd64 register allocator virtual block iterator"));

    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&xregalloc->virtual_blocks, (kefir_hashtree_key_t) block_id, &node);
    if (res == KEFIR_NOT_FOUND) {
        res = KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find virtual block");
    }
    REQUIRE_OK(res);
    ASSIGN_DECL_CAST(struct virtual_block_data *, virtual_block, node->value);

    iter->virtual_block = virtual_block;
    kefir_bucketset_entry_t entry;
    while (iter->virtual_block != NULL) {
        kefir_result_t res = kefir_bucketset_iter(&iter->virtual_block->virtual_registers, &iter->iter, &entry);
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
            ASSIGN_PTR(vreg_idx_ptr, (kefir_asmcmp_virtual_register_index_t) entry);
            return KEFIR_OK;
        } else {
            iter->virtual_block = iter->virtual_block->parent;
        }
    }

    return KEFIR_ITERATOR_END;
}

kefir_result_t kefir_codegen_amd64_xregalloc_block_next(
    struct kefir_codegen_amd64_xregalloc_virtual_block_iterator *iter,
    kefir_asmcmp_virtual_register_index_t *vreg_idx_ptr) {
    REQUIRE(iter != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid amd64 register allocator virtual block iterator"));

    kefir_bucketset_entry_t entry;
    kefir_result_t res = kefir_bucketset_next(&iter->iter, &entry);
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
        ASSIGN_PTR(vreg_idx_ptr, (kefir_asmcmp_virtual_register_index_t) entry);
        return KEFIR_OK;
    }

    iter->virtual_block = iter->virtual_block->parent;
    while (iter->virtual_block != NULL) {
        kefir_result_t res = kefir_bucketset_iter(&iter->virtual_block->virtual_registers, &iter->iter, &entry);
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
            ASSIGN_PTR(vreg_idx_ptr, (kefir_asmcmp_virtual_register_index_t) entry);
            return KEFIR_OK;
        } else {
            iter->virtual_block = iter->virtual_block->parent;
        }
    }

    return KEFIR_ITERATOR_END;
}

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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct mem2reg_reg_state {
    struct kefir_hashtree block_inputs;
    struct kefir_hashtree block_outputs;
};

struct mem2reg_state {
    struct kefir_mem *mem;
    const struct kefir_opt_module *module;
    struct kefir_opt_function *func;
    struct kefir_opt_code_control_flow control_flow;
    struct kefir_hashset promoted_allocs;
    struct kefir_hashtree local_regs;
    struct kefir_list block_queue;
    struct kefir_hashset visited_blocks;

    struct kefir_ir_type *new_locals;
};

static kefir_result_t is_mem2reg_candidate(struct mem2reg_state *state, kefir_opt_instruction_ref_t instr_ref,
                                           kefir_bool_t *skip_candidate) {
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL,
            KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected local allocation optimizer instruction"));

    const struct kefir_ir_type *ir_type =
        kefir_ir_module_get_named_type(state->module->ir_module, instr->operation.parameters.type.type_id);
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));
    const struct kefir_ir_typeentry *local_typeentry =
        kefir_ir_type_at(ir_type, instr->operation.parameters.type.type_index);
    REQUIRE(local_typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to fetch local variable type"));

    *skip_candidate = false;
    switch (local_typeentry->typecode) {
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_INT64:
        case KEFIR_IR_TYPE_INT128:
        case KEFIR_IR_TYPE_FLOAT32:
        case KEFIR_IR_TYPE_FLOAT64:
        case KEFIR_IR_TYPE_LONG_DOUBLE:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
        case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
        case KEFIR_IR_TYPE_BITINT:
        case KEFIR_IR_TYPE_DECIMAL32:
        case KEFIR_IR_TYPE_DECIMAL64:
        case KEFIR_IR_TYPE_DECIMAL128:
            // Intentionally left blank
            break;

        case KEFIR_IR_TYPE_STRUCT:
        case KEFIR_IR_TYPE_ARRAY:
        case KEFIR_IR_TYPE_UNION:
        case KEFIR_IR_TYPE_BITFIELD:
        case KEFIR_IR_TYPE_NONE:
        case KEFIR_IR_TYPE_COUNT:
            *skip_candidate = true;
            break;
    }

    kefir_result_t res;
    struct kefir_opt_instruction_use_iterator use_iter;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(&state->func->code, instr_ref, &use_iter);
         !*skip_candidate && res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, use_iter.use_instr_ref, &use_instr));

        kefir_bool_t reachable;
        REQUIRE_OK(
            kefir_opt_code_control_flow_is_reachable_from_entry(&state->control_flow, use_instr->block_id, &reachable));
        if (!reachable) {
            continue;
        }

        switch (use_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_INT8_LOAD:
            case KEFIR_OPT_OPCODE_INT16_LOAD:
            case KEFIR_OPT_OPCODE_INT32_LOAD:
            case KEFIR_OPT_OPCODE_INT64_LOAD:
            case KEFIR_OPT_OPCODE_INT128_LOAD:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
                *skip_candidate = use_instr->operation.parameters.memory_access.flags.volatile_access;
                break;

            case KEFIR_OPT_OPCODE_INT8_STORE:
            case KEFIR_OPT_OPCODE_INT16_STORE:
            case KEFIR_OPT_OPCODE_INT32_STORE:
            case KEFIR_OPT_OPCODE_INT64_STORE:
            case KEFIR_OPT_OPCODE_INT128_STORE:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
                if (instr_ref == use_instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF]) {
                    *skip_candidate = true;
                } else {
                    *skip_candidate = use_instr->operation.parameters.memory_access.flags.volatile_access;
                }
                break;

            default:
                *skip_candidate = true;
                break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t mem2reg_scan(struct mem2reg_state *state) {
    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&state->func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_ref;
        for (kefir_opt_code_block_instr_head(&state->func->code, block, &instr_ref); instr_ref != KEFIR_ID_NONE;
             kefir_opt_instruction_next_sibling(&state->func->code, instr_ref, &instr_ref)) {

            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));
            if (instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
                kefir_bool_t skip_candidate = false;
                REQUIRE_OK(is_mem2reg_candidate(state, instr_ref, &skip_candidate));
                if (!skip_candidate) {
                    REQUIRE_OK(kefir_hashset_add(state->mem, &state->promoted_allocs, (kefir_hashset_key_t) instr_ref));
                }
            }
        }
    }

    return KEFIR_OK;
}

static kefir_result_t reg_state_for(struct mem2reg_state *state, kefir_opt_instruction_ref_t instr_ref,
                                    struct mem2reg_reg_state **reg_state) {
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&state->local_regs, (kefir_hashtree_key_t) instr_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        *reg_state = KEFIR_MALLOC(state->mem, sizeof(struct mem2reg_reg_state));
        REQUIRE(*reg_state != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate mem2reg register state"));
        res = kefir_hashtree_init(&(*reg_state)->block_inputs, &kefir_hashtree_uint_ops);
        REQUIRE_CHAIN(&res, kefir_hashtree_init(&(*reg_state)->block_outputs, &kefir_hashtree_uint_ops));
        REQUIRE_CHAIN(&res, kefir_hashtree_insert(state->mem, &state->local_regs, (kefir_hashtree_key_t) instr_ref,
                                                  (kefir_hashtree_value_t) *reg_state));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(state->mem, *reg_state);
            *reg_state = NULL;
            return res;
        });
        return KEFIR_OK;
    } else {
        REQUIRE_OK(res);
        *reg_state = (struct mem2reg_reg_state *) node->value;
    }
    return KEFIR_OK;
}

static kefir_result_t assign_empty_value(struct mem2reg_state *state, const struct kefir_ir_typeentry *local_typeentry,
                                         kefir_opt_block_id_t source_block_ref,
                                         kefir_opt_instruction_ref_t *instr_ref) {
    switch (local_typeentry->typecode) {
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_INT64:
            REQUIRE_OK(
                kefir_opt_code_builder_int_constant(state->mem, &state->func->code, source_block_ref, 0, instr_ref));
            break;

        case KEFIR_IR_TYPE_INT128:
            REQUIRE_OK(
                kefir_opt_code_builder_int_constant(state->mem, &state->func->code, source_block_ref, 0, instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_int128_zero_extend_64bits(state->mem, &state->func->code,
                                                                        source_block_ref, *instr_ref, instr_ref));
            break;

        case KEFIR_IR_TYPE_FLOAT32:
            REQUIRE_OK(kefir_opt_code_builder_float32_constant(state->mem, &state->func->code, source_block_ref, 0.0f,
                                                               instr_ref));
            break;

        case KEFIR_IR_TYPE_FLOAT64:
            REQUIRE_OK(kefir_opt_code_builder_float64_constant(state->mem, &state->func->code, source_block_ref, 0.0,
                                                               instr_ref));
            break;

        case KEFIR_IR_TYPE_LONG_DOUBLE:
            REQUIRE_OK(kefir_opt_code_builder_long_double_constant(state->mem, &state->func->code, source_block_ref,
                                                                   0.0L, instr_ref));
            break;

        case KEFIR_IR_TYPE_COMPLEX_FLOAT32: {
            kefir_opt_instruction_ref_t zero_instr_ref;
            REQUIRE_OK(kefir_opt_code_builder_float32_constant(state->mem, &state->func->code, source_block_ref, 0.0f,
                                                               &zero_instr_ref));

            REQUIRE_OK(kefir_opt_code_builder_complex_float32_from(state->mem, &state->func->code, source_block_ref,
                                                                   zero_instr_ref, zero_instr_ref, instr_ref));
        } break;

        case KEFIR_IR_TYPE_COMPLEX_FLOAT64: {
            kefir_opt_instruction_ref_t zero_instr_ref;
            REQUIRE_OK(kefir_opt_code_builder_float64_constant(state->mem, &state->func->code, source_block_ref, 0.0,
                                                               &zero_instr_ref));

            REQUIRE_OK(kefir_opt_code_builder_complex_float64_from(state->mem, &state->func->code, source_block_ref,
                                                                   zero_instr_ref, zero_instr_ref, instr_ref));
        } break;

        case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE: {
            kefir_opt_instruction_ref_t zero_instr_ref;
            REQUIRE_OK(kefir_opt_code_builder_long_double_constant(state->mem, &state->func->code, source_block_ref,
                                                                   0.0L, &zero_instr_ref));

            REQUIRE_OK(kefir_opt_code_builder_complex_long_double_from(state->mem, &state->func->code, source_block_ref,
                                                                       zero_instr_ref, zero_instr_ref, instr_ref));
        } break;

        case KEFIR_IR_TYPE_BITINT: {
            kefir_opt_instruction_ref_t zero_instr_ref;
            REQUIRE_OK(kefir_opt_code_builder_int_constant(state->mem, &state->func->code, source_block_ref, 0,
                                                           &zero_instr_ref));
            REQUIRE_OK(kefir_opt_code_builder_bitint_from_unsigned(state->mem, &state->func->code, source_block_ref,
                                                                   local_typeentry->param, zero_instr_ref, instr_ref));
        } break;

        case KEFIR_IR_TYPE_DECIMAL32:
            REQUIRE_OK(kefir_dfp_require_supported(NULL));
            REQUIRE_OK(kefir_opt_code_builder_decimal32_constant(state->mem, &state->func->code, source_block_ref,
                                                                 kefir_dfp_decimal32_from_int64(0), instr_ref));
            break;

        case KEFIR_IR_TYPE_DECIMAL64:
            REQUIRE_OK(kefir_dfp_require_supported(NULL));
            REQUIRE_OK(kefir_opt_code_builder_decimal64_constant(state->mem, &state->func->code, source_block_ref,
                                                                 kefir_dfp_decimal64_from_int64(0), instr_ref));
            break;

        case KEFIR_IR_TYPE_DECIMAL128:
            REQUIRE_OK(kefir_dfp_require_supported(NULL));
            REQUIRE_OK(kefir_opt_code_builder_decimal128_constant(state->mem, &state->func->code, source_block_ref,
                                                                  kefir_dfp_decimal128_from_int64(0), instr_ref));
            break;

        case KEFIR_IR_TYPE_STRUCT:
        case KEFIR_IR_TYPE_ARRAY:
        case KEFIR_IR_TYPE_UNION:
        case KEFIR_IR_TYPE_BITFIELD:
        case KEFIR_IR_TYPE_NONE:
        case KEFIR_IR_TYPE_COUNT:
            return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected local IR type");
    }
    return KEFIR_OK;
}

static kefir_result_t replace_references(struct mem2reg_state *state, kefir_opt_instruction_ref_t to_ref,
                                         kefir_opt_instruction_ref_t from_ref) {
    REQUIRE_OK(kefir_opt_code_container_replace_references(state->mem, &state->func->code, to_ref, from_ref));

    struct kefir_hashtree_node_iterator iter1, iter2;
    for (struct kefir_hashtree_node *node1 = kefir_hashtree_iter(&state->local_regs, &iter1); node1 != NULL;
         node1 = kefir_hashtree_next(&iter1)) {
        ASSIGN_DECL_CAST(struct mem2reg_reg_state *, reg_state, node1->value);
        for (struct kefir_hashtree_node *node2 = kefir_hashtree_iter(&reg_state->block_outputs, &iter2); node2 != NULL;
             node2 = kefir_hashtree_next(&iter2)) {
            if (node2->value == (kefir_hashtree_value_t) from_ref) {
                node2->value = (kefir_hashtree_value_t) to_ref;
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t mem2reg_pull(struct mem2reg_state *state) {
    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&state->func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        kefir_opt_instruction_ref_t instr_id;
        const struct kefir_opt_instruction *instr = NULL, *addr_instr = NULL;

        for (kefir_opt_code_block_instr_control_head(&state->func->code, block, &instr_id);
             instr_id != KEFIR_ID_NONE;) {

            REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_id, &instr));
            const kefir_opt_block_id_t block_id = instr->block_id;
            kefir_opt_instruction_ref_t replacement_ref = KEFIR_ID_NONE;
            switch (instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_INT8_LOAD:
                case KEFIR_OPT_OPCODE_INT16_LOAD:
                case KEFIR_OPT_OPCODE_INT32_LOAD:
                case KEFIR_OPT_OPCODE_INT64_LOAD:
                case KEFIR_OPT_OPCODE_INT128_LOAD:
                case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
                case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD:
                case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD:
                case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD:
                case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
                case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
                case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
                case KEFIR_OPT_OPCODE_BITINT_LOAD:
                    REQUIRE_OK(kefir_opt_code_container_instr(
                        &state->func->code, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                        &addr_instr));
                    if (addr_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
                        kefir_hashset_has(&state->promoted_allocs, (kefir_hashset_key_t) addr_instr->id)) {
                        const kefir_opt_instruction_ref_t addr_instr_ref = addr_instr->id;
                        struct mem2reg_reg_state *reg_state;
                        REQUIRE_OK(reg_state_for(state, addr_instr_ref, &reg_state));

                        const struct kefir_ir_type *local_type = kefir_ir_module_get_named_type(
                            state->module->ir_module, addr_instr->operation.parameters.type.type_id);
                        REQUIRE(local_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));
                        const struct kefir_ir_typeentry *local_typeentry =
                            kefir_ir_type_at(local_type, addr_instr->operation.parameters.type.type_index);
                        REQUIRE(local_typeentry != NULL,
                                KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to fetch local variable type"));

                        REQUIRE_OK(kefir_opt_code_debug_info_next_instruction_code_reference_of(
                            &state->func->debug_info, instr_id));

                        struct kefir_hashtree_node *node;
                        kefir_result_t res =
                            kefir_hashtree_at(&reg_state->block_outputs, (kefir_hashtree_key_t) block_id, &node);
                        if (res == KEFIR_NOT_FOUND) {
                            if (block_id != state->func->code.entry_point) {
                                kefir_opt_phi_id_t phi_ref;
                                REQUIRE_OK(kefir_opt_code_container_new_phi(state->mem, &state->func->code, block_id,
                                                                            &phi_ref, &replacement_ref));
                                REQUIRE_OK(kefir_hashtree_insert(state->mem, &reg_state->block_inputs,
                                                                 (kefir_hashtree_key_t) block_id,
                                                                 (kefir_hashtree_value_t) phi_ref));
                            } else {
                                REQUIRE_OK(assign_empty_value(state, local_typeentry, block_id, &replacement_ref));
                            }
                            REQUIRE_OK(kefir_hashtree_insert(state->mem, &reg_state->block_outputs,
                                                             (kefir_hashtree_key_t) block_id,
                                                             (kefir_hashtree_value_t) replacement_ref));
                            REQUIRE_OK(kefir_opt_code_debug_info_add_allocation_placement(
                                state->mem, &state->func->debug_info, addr_instr_ref, replacement_ref));
                        } else {
                            replacement_ref = node->value;
                        }

                        if (replacement_ref != instr_id) {
                            switch (local_typeentry->typecode) {
                                case KEFIR_IR_TYPE_INT8:
                                case KEFIR_IR_TYPE_INT16:
                                case KEFIR_IR_TYPE_INT32:
                                case KEFIR_IR_TYPE_INT64:
                                case KEFIR_IR_TYPE_INT128:
                                    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_id, &instr));
                                    switch (instr->operation.opcode) {
                                        case KEFIR_OPT_OPCODE_INT8_LOAD:
                                            if (instr->operation.parameters.memory_access.flags.load_extension ==
                                                KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND) {
                                                REQUIRE_OK(kefir_opt_code_builder_int64_sign_extend_8bits(
                                                    state->mem, &state->func->code, block_id, replacement_ref,
                                                    &replacement_ref));
                                            } else if (instr->operation.parameters.memory_access.flags.load_extension ==
                                                       KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND) {
                                                REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_8bits(
                                                    state->mem, &state->func->code, block_id, replacement_ref,
                                                    &replacement_ref));
                                            }
                                            break;

                                        case KEFIR_OPT_OPCODE_INT16_LOAD:
                                            if (instr->operation.parameters.memory_access.flags.load_extension ==
                                                KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND) {
                                                REQUIRE_OK(kefir_opt_code_builder_int64_sign_extend_16bits(
                                                    state->mem, &state->func->code, block_id, replacement_ref,
                                                    &replacement_ref));
                                            } else if (instr->operation.parameters.memory_access.flags.load_extension ==
                                                       KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND) {
                                                REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_16bits(
                                                    state->mem, &state->func->code, block_id, replacement_ref,
                                                    &replacement_ref));
                                            }
                                            break;

                                        case KEFIR_OPT_OPCODE_INT32_LOAD:
                                            if (instr->operation.parameters.memory_access.flags.load_extension ==
                                                KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND) {
                                                REQUIRE_OK(kefir_opt_code_builder_int64_sign_extend_32bits(
                                                    state->mem, &state->func->code, block_id, replacement_ref,
                                                    &replacement_ref));
                                            } else if (instr->operation.parameters.memory_access.flags.load_extension ==
                                                       KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND) {
                                                REQUIRE_OK(kefir_opt_code_builder_int64_zero_extend_32bits(
                                                    state->mem, &state->func->code, block_id, replacement_ref,
                                                    &replacement_ref));
                                            }
                                            break;

                                        case KEFIR_OPT_OPCODE_INT64_LOAD:
                                        case KEFIR_OPT_OPCODE_INT128_LOAD:
                                            // Intentionally left blank
                                            break;

                                        default:
                                            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                                                                   "Unexpected instruction opcode");
                                    }
                                    break;

                                case KEFIR_IR_TYPE_BITINT:
                                    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_id, &instr));
                                    switch (instr->operation.opcode) {
                                        case KEFIR_OPT_OPCODE_BITINT_LOAD:
                                            if (instr->operation.parameters.bitint_memflags.load_extension ==
                                                KEFIR_OPT_MEMORY_LOAD_SIGN_EXTEND) {
                                                REQUIRE_OK(kefir_opt_code_builder_bitint_cast_signed(
                                                    state->mem, &state->func->code, block_id,
                                                    instr->operation.parameters.bitwidth, local_typeentry->param,
                                                    replacement_ref, &replacement_ref));
                                            } else if (instr->operation.parameters.bitint_memflags.load_extension ==
                                                       KEFIR_OPT_MEMORY_LOAD_ZERO_EXTEND) {
                                                REQUIRE_OK(kefir_opt_code_builder_bitint_cast_unsigned(
                                                    state->mem, &state->func->code, block_id,
                                                    instr->operation.parameters.bitwidth, local_typeentry->param,
                                                    replacement_ref, &replacement_ref));
                                            }
                                            break;

                                        default:
                                            return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR,
                                                                   "Unexpected instruction opcode");
                                    }
                                    break;

                                default:
                                    // Intentionally left blank
                                    break;
                            }

                            REQUIRE_OK(replace_references(state, replacement_ref, instr_id));
                            kefir_opt_instruction_ref_t prev_instr_id = instr_id;
                            REQUIRE_OK(kefir_opt_instruction_next_control(&state->func->code, instr_id, &instr_id));
                            REQUIRE_OK(kefir_opt_code_container_drop_control(&state->func->code, prev_instr_id));
                            REQUIRE_OK(
                                kefir_opt_code_container_drop_instr(state->mem, &state->func->code, prev_instr_id));
                        } else {
                            REQUIRE_OK(kefir_opt_instruction_next_control(&state->func->code, instr_id, &instr_id));
                        }

                        REQUIRE_OK(kefir_opt_code_debug_info_next_instruction_code_reference(
                            &state->func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_CODE_REF_NONE));
                    } else {
                        REQUIRE_OK(kefir_opt_instruction_next_control(&state->func->code, instr_id, &instr_id));
                    }
                    break;

                case KEFIR_OPT_OPCODE_INT8_STORE:
                case KEFIR_OPT_OPCODE_INT16_STORE:
                case KEFIR_OPT_OPCODE_INT32_STORE:
                case KEFIR_OPT_OPCODE_INT64_STORE:
                case KEFIR_OPT_OPCODE_INT128_STORE:
                case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
                case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
                case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
                case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
                case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
                case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
                case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
                case KEFIR_OPT_OPCODE_BITINT_STORE:
                    REQUIRE_OK(kefir_opt_code_container_instr(
                        &state->func->code, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                        &addr_instr));
                    if (addr_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
                        kefir_hashset_has(&state->promoted_allocs, (kefir_hashset_key_t) addr_instr->id)) {
                        const kefir_opt_instruction_ref_t addr_instr_ref = addr_instr->id;
                        struct mem2reg_reg_state *reg_state;
                        REQUIRE_OK(reg_state_for(state, addr_instr_ref, &reg_state));

                        kefir_opt_instruction_ref_t replacement_ref =
                            instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF];
                        struct kefir_hashtree_node *node;
                        kefir_result_t res =
                            kefir_hashtree_at(&reg_state->block_outputs, (kefir_hashtree_key_t) block_id, &node);
                        if (res == KEFIR_NOT_FOUND) {
                            REQUIRE_OK(kefir_hashtree_insert(state->mem, &reg_state->block_outputs,
                                                             (kefir_hashtree_key_t) block_id,
                                                             (kefir_hashtree_value_t) replacement_ref));
                        } else {
                            node->value = replacement_ref;
                        }

                        kefir_opt_instruction_ref_t prev_instr_id = instr_id;
                        REQUIRE_OK(kefir_opt_instruction_next_control(&state->func->code, instr_id, &instr_id));
                        REQUIRE_OK(kefir_opt_code_container_drop_control(&state->func->code, prev_instr_id));
                        REQUIRE_OK(kefir_opt_code_container_drop_instr(state->mem, &state->func->code, prev_instr_id));

                        REQUIRE_OK(kefir_opt_code_debug_info_add_allocation_placement(
                            state->mem, &state->func->debug_info, addr_instr_ref, replacement_ref));
                    } else {
                        REQUIRE_OK(kefir_opt_instruction_next_control(&state->func->code, instr_id, &instr_id));
                    }
                    break;

                default:
                    REQUIRE_OK(kefir_opt_instruction_next_control(&state->func->code, instr_id, &instr_id));
                    break;
            }
        }
    }

    return KEFIR_OK;
}

static kefir_result_t mem2reg_link_blocks(struct mem2reg_state *state, kefir_opt_instruction_ref_t instr_ref,
                                          struct mem2reg_reg_state *reg_state, kefir_opt_phi_id_t phi_ref,
                                          kefir_opt_block_id_t source_block_ref) {
    kefir_opt_instruction_ref_t source_instr_ref, existing_ref;
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&reg_state->block_outputs, (kefir_hashtree_key_t) source_block_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        if (source_block_ref != state->func->code.entry_point) {
            kefir_opt_phi_id_t source_phi_ref;
            REQUIRE_OK(kefir_opt_code_container_new_phi(state->mem, &state->func->code, source_block_ref,
                                                        &source_phi_ref, &source_instr_ref));
            REQUIRE_OK(kefir_hashtree_insert(state->mem, &reg_state->block_inputs,
                                             (kefir_hashtree_key_t) source_block_ref,
                                             (kefir_hashtree_value_t) source_phi_ref));
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->block_queue, kefir_list_tail(&state->block_queue),
                                               (void *) (kefir_uptr_t) source_block_ref));
        } else {
            const struct kefir_opt_instruction *addr_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &addr_instr));
            const struct kefir_ir_type *local_type =
                kefir_ir_module_get_named_type(state->module->ir_module, addr_instr->operation.parameters.type.type_id);
            REQUIRE(local_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));
            const struct kefir_ir_typeentry *local_typeentry =
                kefir_ir_type_at(local_type, addr_instr->operation.parameters.type.type_index);
            REQUIRE(local_typeentry != NULL,
                    KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to fetch local variable type"));
            REQUIRE_OK(assign_empty_value(state, local_typeentry, source_block_ref, &source_instr_ref));
        }
        REQUIRE_OK(kefir_hashtree_insert(state->mem, &reg_state->block_outputs, (kefir_hashtree_key_t) source_block_ref,
                                         (kefir_hashtree_value_t) source_instr_ref));
    } else {
        REQUIRE_OK(res);
        source_instr_ref = (kefir_opt_instruction_ref_t) node->value;
    }

    res = kefir_opt_code_container_phi_link_for(&state->func->code, phi_ref, source_block_ref, &existing_ref);
    if (res != KEFIR_NOT_FOUND) {
        REQUIRE_OK(res);
        REQUIRE(source_instr_ref == existing_ref,
                KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected existing phi node reference"));
    } else {
        REQUIRE_OK(kefir_opt_code_container_phi_attach(state->mem, &state->func->code, phi_ref, source_block_ref,
                                                       source_instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t mem2reg_propagate_impl(struct mem2reg_state *state, kefir_opt_instruction_ref_t instr_ref,
                                             struct mem2reg_reg_state *reg_state) {
    for (const struct kefir_list_entry *iter = kefir_list_head(&state->block_queue); iter != NULL;
         kefir_list_pop(state->mem, &state->block_queue, (struct kefir_list_entry *) iter),
                                       iter = kefir_list_head(&state->block_queue)) {

        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter->value);
        if (kefir_hashset_has(&state->visited_blocks, (kefir_hashset_key_t) block_id)) {
            continue;
        }
        REQUIRE_OK(kefir_hashset_add(state->mem, &state->visited_blocks, (kefir_hashset_key_t) block_id));

        struct kefir_hashtree_node *node;
        kefir_result_t res = kefir_hashtree_at(&reg_state->block_inputs, (kefir_hashtree_key_t) block_id, &node);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(kefir_opt_phi_id_t, phi_ref, node->value);
        REQUIRE(phi_ref != KEFIR_ID_NONE, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected optimizer phi reference"));

        struct kefir_hashset_iterator iter;
        kefir_hashset_key_t entry;
        for (res = kefir_hashset_iter(&state->control_flow.blocks[block_id].predecessors, &iter, &entry);
             res == KEFIR_OK; res = kefir_hashset_next(&iter, &entry)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, source_block_ref, entry);
            REQUIRE_OK(mem2reg_link_blocks(state, instr_ref, reg_state, phi_ref, source_block_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t mem2reg_propagate(struct mem2reg_state *state) {
    struct kefir_hashtree_node_iterator iter, inputs_iter;
    for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&state->local_regs, &iter); node != NULL;
         node = kefir_hashtree_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, node->key);
        ASSIGN_DECL_CAST(struct mem2reg_reg_state *, reg_state, node->value);

        for (const struct kefir_hashtree_node *inputs_node =
                 kefir_hashtree_iter(&reg_state->block_inputs, &inputs_iter);
             inputs_node != NULL; inputs_node = kefir_hashtree_next(&inputs_iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, inputs_node->key);

            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->block_queue, kefir_list_tail(&state->block_queue),
                                               (void *) (kefir_uptr_t) block_id));
        }
        REQUIRE_OK(kefir_hashset_clear(state->mem, &state->visited_blocks));
        REQUIRE_OK(mem2reg_propagate_impl(state, instr_ref, reg_state));
    }
    return KEFIR_OK;
}

static kefir_result_t free_mem2reg_reg_state(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                             kefir_hashtree_key_t key, kefir_hashtree_value_t value, void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct mem2reg_reg_state *, state, value);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid mem2reg register state"));

    REQUIRE_OK(kefir_hashtree_free(mem, &state->block_inputs));
    REQUIRE_OK(kefir_hashtree_free(mem, &state->block_outputs));
    memset(state, 0, sizeof(struct mem2reg_reg_state));
    KEFIR_FREE(mem, state);
    return KEFIR_OK;
}

static kefir_result_t mem2reg_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                    struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                    const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct mem2reg_state state = {.mem = mem, .module = module, .func = func};
    REQUIRE_OK(kefir_hashset_init(&state.promoted_allocs, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&state.local_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&state.local_regs, free_mem2reg_reg_state, NULL));
    REQUIRE_OK(kefir_list_init(&state.block_queue));
    REQUIRE_OK(kefir_hashset_init(&state.visited_blocks, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_opt_code_control_flow_init(&state.control_flow));

    kefir_result_t res = kefir_opt_code_control_flow_build(mem, &state.control_flow, &func->code);
    REQUIRE_CHAIN(&res, mem2reg_scan(&state));
    REQUIRE_CHAIN(&res, mem2reg_pull(&state));
    REQUIRE_CHAIN(&res, mem2reg_propagate(&state));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        kefir_hashset_free(mem, &state.visited_blocks);
        kefir_list_free(mem, &state.block_queue);
        kefir_hashtree_free(mem, &state.local_regs);
        kefir_hashset_free(mem, &state.promoted_allocs);
        return res;
    });

    res = kefir_opt_code_control_flow_free(mem, &state.control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.visited_blocks);
        kefir_list_free(mem, &state.block_queue);
        kefir_hashtree_free(mem, &state.local_regs);
        kefir_hashset_free(mem, &state.promoted_allocs);
        return res;
    });
    res = kefir_hashset_free(mem, &state.visited_blocks);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.block_queue);
        kefir_hashtree_free(mem, &state.local_regs);
        kefir_hashset_free(mem, &state.promoted_allocs);
        return res;
    });
    res = kefir_list_free(mem, &state.block_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &state.local_regs);
        kefir_hashset_free(mem, &state.promoted_allocs);
        return res;
    });
    res = kefir_hashtree_free(mem, &state.local_regs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.promoted_allocs);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &state.promoted_allocs));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassMem2Reg = {
    .name = "mem2reg", .apply = mem2reg_apply, .payload = NULL};

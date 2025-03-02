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

#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/builder.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/core/hashtreeset.h"
#include <string.h>
#include <stdio.h>

struct mem2reg_reg_state {
    struct kefir_hashtree block_inputs;
    struct kefir_hashtree block_outputs;
};

struct mem2reg_block_predecessors {
    struct kefir_hashtreeset preds;
};

struct mem2reg_state {
    struct kefir_mem *mem;
    const struct kefir_opt_module *module;
    struct kefir_opt_function *func;
    struct kefir_hashtreeset addressed_locals;
    struct kefir_hashtreeset scalar_local_candidates;
    struct kefir_hashtree block_predecessors;
    struct kefir_hashtree local_regs;
    struct kefir_list block_queue;
    struct kefir_hashtreeset visited_blocks;

    struct kefir_ir_type *new_locals;
};

static kefir_result_t mark_local_addressed(struct mem2reg_state *state, struct kefir_hashtreeset *visited,
                                           kefir_opt_instruction_ref_t instr_ref) {
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));
    if (instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL) {
        REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->addressed_locals, (kefir_hashtreeset_entry_t) instr->id));
        REQUIRE_OK(kefir_hashtreeset_delete(state->mem, &state->scalar_local_candidates,
                                            (kefir_hashtreeset_entry_t) instr->id));
        return KEFIR_OK;
    } else if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
        kefir_result_t res;
        const struct kefir_opt_phi_node *phi_node;
        struct kefir_opt_phi_node_link_iterator iter;

        REQUIRE_OK(kefir_opt_code_container_phi(&state->func->code, instr->operation.parameters.phi_ref, &phi_node));
        kefir_opt_instruction_ref_t src_instr_ref;

        struct kefir_hashtreeset visited_set;
        struct kefir_hashtreeset *visited_ptr = visited;
        if (visited == NULL) {
            REQUIRE_OK(kefir_hashtreeset_init(&visited_set, &kefir_hashtree_uint_ops));
            visited_ptr = &visited_set;
        }

        for (res = kefir_opt_phi_node_link_iter(phi_node, &iter, NULL, &src_instr_ref); res == KEFIR_OK;) {
            if (kefir_hashtreeset_has(visited_ptr, (kefir_hashtreeset_entry_t) src_instr_ref)) {
                REQUIRE_CHAIN(&res, kefir_opt_phi_node_link_next(&iter, NULL, &src_instr_ref));
                continue;
            }
            REQUIRE_CHAIN(&res, kefir_hashtreeset_add(state->mem, visited_ptr, (kefir_hashtreeset_entry_t) instr_ref));
            REQUIRE_CHAIN(&res, mark_local_addressed(state, visited_ptr, src_instr_ref));
            REQUIRE_CHAIN(&res, kefir_opt_phi_node_link_next(&iter, NULL, &src_instr_ref));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_ELSE(res == KEFIR_OK, {
                if (visited == NULL) {
                    kefir_hashtreeset_free(state->mem, &visited_set);
                }
                return res;
            });
        }

        if (visited == NULL) {
            REQUIRE_OK(kefir_hashtreeset_free(state->mem, &visited_set));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t extract_local_inputs(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct mem2reg_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid mem2reg optimization pass state"));

    REQUIRE_OK(mark_local_addressed(state, NULL, instr_ref));
    return KEFIR_OK;
}

static kefir_result_t mark_scalar_candidate(struct mem2reg_state *state, kefir_opt_instruction_ref_t instr_ref) {
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));
    REQUIRE(instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL,
            KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected optimizer instruction opcode"));

    const struct kefir_ir_type *ir_type =
        kefir_ir_module_get_named_type(state->module->ir_module, instr->operation.parameters.type.type_id);
    REQUIRE(ir_type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to find IR type"));
    const struct kefir_ir_typeentry *local_typeentry =
        kefir_ir_type_at(ir_type, instr->operation.parameters.type.type_index);
    REQUIRE(local_typeentry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unable to fetch local variable type"));

    switch (local_typeentry->typecode) {
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
        case KEFIR_IR_TYPE_SHORT:
        case KEFIR_IR_TYPE_INT:
        case KEFIR_IR_TYPE_LONG:
        case KEFIR_IR_TYPE_WORD:
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_INT64:
        case KEFIR_IR_TYPE_FLOAT32:
        case KEFIR_IR_TYPE_FLOAT64:
            REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->scalar_local_candidates,
                                             (kefir_hashtreeset_entry_t) instr_ref));
            break;

        case KEFIR_IR_TYPE_STRUCT:
        case KEFIR_IR_TYPE_ARRAY:
        case KEFIR_IR_TYPE_UNION:
        case KEFIR_IR_TYPE_LONG_DOUBLE:
        case KEFIR_IR_TYPE_BITS:
        case KEFIR_IR_TYPE_BUILTIN:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
        case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
        case KEFIR_IR_TYPE_NONE:
        case KEFIR_IR_TYPE_COUNT:
            // Intentionally left blank
            break;
    }
    return KEFIR_OK;
}

static kefir_result_t add_block_predecessor(struct mem2reg_state *state, kefir_opt_block_id_t source_block_ref,
                                            kefir_opt_block_id_t target_block_ref) {
    struct mem2reg_block_predecessors *preds;
    struct kefir_hashtree_node *node;
    kefir_result_t res = kefir_hashtree_at(&state->block_predecessors, (kefir_hashtree_key_t) target_block_ref, &node);
    if (res == KEFIR_NOT_FOUND) {
        preds = KEFIR_MALLOC(state->mem, sizeof(struct mem2reg_block_predecessors));
        REQUIRE(preds != NULL,
                KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate mem2reg block predecessors"));
        res = kefir_hashtreeset_init(&preds->preds, &kefir_hashtree_uint_ops);
        REQUIRE_CHAIN(
            &res, kefir_hashtree_insert(state->mem, &state->block_predecessors, (kefir_hashtree_key_t) target_block_ref,
                                        (kefir_hashtree_value_t) preds));
        REQUIRE_ELSE(res == KEFIR_OK, {
            KEFIR_FREE(state->mem, preds);
            return res;
        });
    } else {
        REQUIRE_OK(res);
        preds = (struct mem2reg_block_predecessors *) node->value;
    }

    REQUIRE_OK(kefir_hashtreeset_add(state->mem, &preds->preds, (kefir_hashtreeset_entry_t) source_block_ref));
    return KEFIR_OK;
}

static kefir_result_t mem2reg_scan(struct mem2reg_state *state) {
    struct kefir_opt_code_container_iterator iter;
    for (struct kefir_opt_code_block *block = kefir_opt_code_container_iter(&state->func->code, &iter); block != NULL;
         block = kefir_opt_code_container_next(&iter)) {

        const struct kefir_opt_instruction *instr = NULL, *addr_instr = NULL;

        if (!kefir_hashtreeset_empty(&block->public_labels)) {
            return KEFIR_YIELD;
        }

        kefir_opt_instruction_ref_t instr_ref;
        for (kefir_opt_code_block_instr_head(&state->func->code, block, &instr_ref); instr_ref != KEFIR_ID_NONE;
             kefir_opt_instruction_next_sibling(&state->func->code, instr_ref, &instr_ref)) {

            REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));

            switch (instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_PHI:
                    REQUIRE_OK(mark_local_addressed(state, NULL, instr->id));
                    break;

                case KEFIR_OPT_OPCODE_INT8_LOAD:
                case KEFIR_OPT_OPCODE_INT16_LOAD:
                case KEFIR_OPT_OPCODE_INT32_LOAD:
                case KEFIR_OPT_OPCODE_INT64_LOAD:
                    REQUIRE_OK(kefir_opt_code_container_instr(
                        &state->func->code, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                        &addr_instr));
                    if (!instr->operation.parameters.memory_access.flags.volatile_access &&
                        addr_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
                        !kefir_hashtreeset_has(&state->addressed_locals, addr_instr->id)) {
                        REQUIRE_OK(mark_scalar_candidate(state, addr_instr->id));
                    } else {
                        REQUIRE_OK(mark_local_addressed(state, NULL, addr_instr->id));
                    }
                    break;

                case KEFIR_OPT_OPCODE_INT8_STORE:
                case KEFIR_OPT_OPCODE_INT16_STORE:
                case KEFIR_OPT_OPCODE_INT32_STORE:
                case KEFIR_OPT_OPCODE_INT64_STORE:
                    REQUIRE_OK(kefir_opt_code_container_instr(
                        &state->func->code, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                        &addr_instr));
                    if (!instr->operation.parameters.memory_access.flags.volatile_access &&
                        addr_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
                        !kefir_hashtreeset_has(&state->addressed_locals, addr_instr->id)) {
                        REQUIRE_OK(mark_scalar_candidate(state, addr_instr->id));
                    } else if (instr->operation.parameters.memory_access.flags.volatile_access) {
                        REQUIRE_OK(mark_local_addressed(state, NULL, addr_instr->id));
                    }
                    REQUIRE_OK(mark_local_addressed(
                        state, NULL, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_VALUE_REF]));
                    break;

                case KEFIR_OPT_OPCODE_JUMP:
                    REQUIRE_OK(
                        add_block_predecessor(state, block->id, instr->operation.parameters.branch.target_block));
                    REQUIRE_OK(kefir_opt_instruction_extract_inputs(&state->func->code, instr, true,
                                                                    extract_local_inputs, state));
                    break;

                case KEFIR_OPT_OPCODE_BRANCH:
                case KEFIR_OPT_OPCODE_BRANCH_COMPARE:
                    REQUIRE_OK(
                        add_block_predecessor(state, block->id, instr->operation.parameters.branch.target_block));
                    REQUIRE_OK(
                        add_block_predecessor(state, block->id, instr->operation.parameters.branch.alternative_block));
                    REQUIRE_OK(kefir_opt_instruction_extract_inputs(&state->func->code, instr, true,
                                                                    extract_local_inputs, state));
                    break;

                case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
                    const struct kefir_opt_inline_assembly_node *inline_asm;
                    REQUIRE_OK(kefir_opt_code_container_inline_assembly(
                        &state->func->code, instr->operation.parameters.inline_asm_ref, &inline_asm));

                    if (!kefir_hashtree_empty(&inline_asm->jump_targets)) {
                        REQUIRE_OK(add_block_predecessor(state, block->id, inline_asm->default_jump_target));

                        struct kefir_hashtree_node_iterator iter;
                        for (const struct kefir_hashtree_node *node =
                                 kefir_hashtree_iter(&inline_asm->jump_targets, &iter);
                             node != NULL; node = kefir_hashtree_next(&iter)) {

                            ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
                            REQUIRE_OK(add_block_predecessor(state, block->id, target_block));
                        }
                    }
                    REQUIRE_OK(kefir_opt_instruction_extract_inputs(&state->func->code, instr, true,
                                                                    extract_local_inputs, state));
                } break;

                case KEFIR_OPT_OPCODE_IJUMP:
                    return KEFIR_YIELD;

                default:
                    REQUIRE_OK(kefir_opt_instruction_extract_inputs(&state->func->code, instr, true,
                                                                    extract_local_inputs, state));
                    break;
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
        case KEFIR_IR_TYPE_BOOL:
        case KEFIR_IR_TYPE_CHAR:
        case KEFIR_IR_TYPE_SHORT:
        case KEFIR_IR_TYPE_INT:
        case KEFIR_IR_TYPE_LONG:
        case KEFIR_IR_TYPE_WORD:
        case KEFIR_IR_TYPE_INT8:
        case KEFIR_IR_TYPE_INT16:
        case KEFIR_IR_TYPE_INT32:
        case KEFIR_IR_TYPE_INT64:
            REQUIRE_OK(
                kefir_opt_code_builder_int_constant(state->mem, &state->func->code, source_block_ref, 0, instr_ref));
            break;

        case KEFIR_IR_TYPE_FLOAT32:
            REQUIRE_OK(kefir_opt_code_builder_float32_constant(state->mem, &state->func->code, source_block_ref, 0.0f,
                                                               instr_ref));
            break;

        case KEFIR_IR_TYPE_FLOAT64:
            REQUIRE_OK(kefir_opt_code_builder_float64_constant(state->mem, &state->func->code, source_block_ref, 0.0,
                                                               instr_ref));
            break;

        case KEFIR_IR_TYPE_STRUCT:
        case KEFIR_IR_TYPE_ARRAY:
        case KEFIR_IR_TYPE_UNION:
        case KEFIR_IR_TYPE_LONG_DOUBLE:
        case KEFIR_IR_TYPE_BITS:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT32:
        case KEFIR_IR_TYPE_COMPLEX_FLOAT64:
        case KEFIR_IR_TYPE_COMPLEX_LONG_DOUBLE:
        case KEFIR_IR_TYPE_BUILTIN:
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

        for (kefir_opt_code_block_instr_head(&state->func->code, block, &instr_id); instr_id != KEFIR_ID_NONE;) {

            REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_id, &instr));
            const kefir_opt_block_id_t block_id = instr->block_id;
            kefir_opt_instruction_ref_t replacement_ref = KEFIR_ID_NONE;
            switch (instr->operation.opcode) {
                case KEFIR_OPT_OPCODE_INT8_LOAD:
                case KEFIR_OPT_OPCODE_INT16_LOAD:
                case KEFIR_OPT_OPCODE_INT32_LOAD:
                case KEFIR_OPT_OPCODE_INT64_LOAD:
                    REQUIRE_OK(kefir_opt_code_container_instr(
                        &state->func->code, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                        &addr_instr));
                    if (addr_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
                        kefir_hashtreeset_has(&state->scalar_local_candidates,
                                              (kefir_hashtreeset_entry_t) addr_instr->id)) {
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

                        REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor_of(
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
                            REQUIRE_OK(kefir_opt_code_debug_info_add_local_variable_ref(
                                state->mem, &state->func->debug_info, addr_instr_ref, replacement_ref));
                        } else {
                            replacement_ref = node->value;
                        }

                        switch (local_typeentry->typecode) {
                            case KEFIR_IR_TYPE_BOOL:
                            case KEFIR_IR_TYPE_CHAR:
                            case KEFIR_IR_TYPE_SHORT:
                            case KEFIR_IR_TYPE_INT:
                            case KEFIR_IR_TYPE_LONG:
                            case KEFIR_IR_TYPE_WORD:
                            case KEFIR_IR_TYPE_INT8:
                            case KEFIR_IR_TYPE_INT16:
                            case KEFIR_IR_TYPE_INT32:
                            case KEFIR_IR_TYPE_INT64:
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
                                        // Intentionally left blank
                                        break;

                                    default:
                                        return KEFIR_SET_ERROR(KEFIR_INTERNAL_ERROR, "Unexpected instruction opcode");
                                }

                            default:
                                // Intentionally left blank
                                break;
                        }

                        REQUIRE_OK(replace_references(state, replacement_ref, instr_id));
                        kefir_opt_instruction_ref_t prev_instr_id = instr_id;
                        REQUIRE_OK(kefir_opt_instruction_next_sibling(&state->func->code, instr_id, &instr_id));
                        REQUIRE_OK(kefir_opt_code_container_drop_control(&state->func->code, prev_instr_id));
                        REQUIRE_OK(kefir_opt_code_container_drop_instr(state->mem, &state->func->code, prev_instr_id));

                        REQUIRE_OK(kefir_opt_code_debug_info_set_instruction_location_cursor(
                            &state->func->debug_info, KEFIR_OPT_CODE_DEBUG_INSTRUCTION_LOCATION_NONE));
                    } else {
                        REQUIRE_OK(kefir_opt_instruction_next_sibling(&state->func->code, instr_id, &instr_id));
                    }
                    break;

                case KEFIR_OPT_OPCODE_INT8_STORE:
                case KEFIR_OPT_OPCODE_INT16_STORE:
                case KEFIR_OPT_OPCODE_INT32_STORE:
                case KEFIR_OPT_OPCODE_INT64_STORE:
                    REQUIRE_OK(kefir_opt_code_container_instr(
                        &state->func->code, instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                        &addr_instr));
                    if (addr_instr->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL &&
                        kefir_hashtreeset_has(&state->scalar_local_candidates,
                                              (kefir_hashtreeset_entry_t) addr_instr->id)) {
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
                        REQUIRE_OK(kefir_opt_code_container_drop_control(&state->func->code, instr_id));
                        REQUIRE_OK(kefir_opt_instruction_next_sibling(&state->func->code, instr_id, &instr_id));
                        REQUIRE_OK(kefir_opt_code_container_drop_instr(state->mem, &state->func->code, prev_instr_id));

                        REQUIRE_OK(kefir_opt_code_debug_info_add_local_variable_ref(
                            state->mem, &state->func->debug_info, addr_instr_ref, replacement_ref));
                    } else {
                        REQUIRE_OK(kefir_opt_instruction_next_sibling(&state->func->code, instr_id, &instr_id));
                    }
                    break;

                default:
                    REQUIRE_OK(kefir_opt_instruction_next_sibling(&state->func->code, instr_id, &instr_id));
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
        if (kefir_hashtreeset_has(&state->visited_blocks, (kefir_hashtreeset_entry_t) block_id)) {
            continue;
        }
        REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->visited_blocks, (kefir_hashtreeset_entry_t) block_id));

        struct kefir_hashtree_node *node;
        kefir_result_t res = kefir_hashtree_at(&reg_state->block_inputs, (kefir_hashtree_key_t) block_id, &node);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(kefir_opt_phi_id_t, phi_ref, node->value);

        res = kefir_hashtree_at(&state->block_predecessors, (kefir_hashtree_key_t) block_id, &node);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        ASSIGN_DECL_CAST(struct mem2reg_block_predecessors *, preds, node->value);

        struct kefir_hashtreeset_iterator source_block_iter;
        for (res = kefir_hashtreeset_iter(&preds->preds, &source_block_iter); res == KEFIR_OK;
             res = kefir_hashtreeset_next(&source_block_iter)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, source_block_ref, source_block_iter.entry);
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
        REQUIRE_OK(kefir_hashtreeset_clean(state->mem, &state->visited_blocks));
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

static kefir_result_t free_mem2reg_block_predecessors(struct kefir_mem *mem, struct kefir_hashtree *tree,
                                                      kefir_hashtree_key_t key, kefir_hashtree_value_t value,
                                                      void *payload) {
    UNUSED(tree);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct mem2reg_block_predecessors *, state, value);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid mem2reg block predecessors"));

    REQUIRE_OK(kefir_hashtreeset_free(mem, &state->preds));
    memset(state, 0, sizeof(struct mem2reg_block_predecessors));
    KEFIR_FREE(mem, state);
    return KEFIR_OK;
}

static kefir_result_t mem2reg_apply(struct kefir_mem *mem, const struct kefir_opt_module *module,
                                    struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                    const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct mem2reg_state state = {.mem = mem, .module = module, .func = func};
    REQUIRE_OK(kefir_hashtreeset_init(&state.addressed_locals, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.scalar_local_candidates, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_init(&state.local_regs, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&state.local_regs, free_mem2reg_reg_state, NULL));
    REQUIRE_OK(kefir_hashtree_init(&state.block_predecessors, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtree_on_removal(&state.block_predecessors, free_mem2reg_block_predecessors, NULL));
    REQUIRE_OK(kefir_list_init(&state.block_queue));
    REQUIRE_OK(kefir_hashtreeset_init(&state.visited_blocks, &kefir_hashtree_uint_ops));

    kefir_result_t res = mem2reg_scan(&state);
    REQUIRE_CHAIN(&res, mem2reg_pull(&state));
    REQUIRE_CHAIN(&res, mem2reg_propagate(&state));
    if (res == KEFIR_YIELD) {
        res = KEFIR_OK;
    }
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &state.visited_blocks);
        kefir_list_free(mem, &state.block_queue);
        kefir_hashtree_free(mem, &state.local_regs);
        kefir_hashtreeset_free(mem, &state.addressed_locals);
        kefir_hashtreeset_free(mem, &state.scalar_local_candidates);
        return res;
    });

    res = kefir_hashtreeset_free(mem, &state.visited_blocks);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.block_queue);
        kefir_hashtree_free(mem, &state.block_predecessors);
        kefir_hashtree_free(mem, &state.local_regs);
        kefir_hashtreeset_free(mem, &state.addressed_locals);
        kefir_hashtreeset_free(mem, &state.scalar_local_candidates);
        return res;
    });
    res = kefir_list_free(mem, &state.block_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &state.block_predecessors);
        kefir_hashtree_free(mem, &state.local_regs);
        kefir_hashtreeset_free(mem, &state.addressed_locals);
        kefir_hashtreeset_free(mem, &state.scalar_local_candidates);
        return res;
    });
    res = kefir_hashtree_free(mem, &state.block_predecessors);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtree_free(mem, &state.local_regs);
        kefir_hashtreeset_free(mem, &state.addressed_locals);
        kefir_hashtreeset_free(mem, &state.scalar_local_candidates);
        return res;
    });
    res = kefir_hashtree_free(mem, &state.local_regs);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &state.addressed_locals);
        kefir_hashtreeset_free(mem, &state.scalar_local_candidates);
        return res;
    });
    res = kefir_hashtreeset_free(mem, &state.addressed_locals);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &state.scalar_local_candidates);
        return res;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &state.scalar_local_candidates));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassMem2Reg = {
    .name = "mem2reg", .apply = mem2reg_apply, .payload = NULL};

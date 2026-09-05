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
#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

#define MAX_MERGE_CHAIN 64

struct merge_state {
    struct kefir_opt_code_control_flow control_flow;
    struct kefir_list merge_order;
    struct kefir_list stack;
    kefir_uint8_t *block_merges;
};

static kefir_result_t collect_merge_order(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct merge_state *state) {
    REQUIRE_OK(kefir_list_clear(mem, &state->stack));
    REQUIRE_OK(kefir_list_insert_after(mem, &state->stack, NULL, (void *) (kefir_uptr_t) func->code.entry_point));

    for (struct kefir_list_entry *iter = kefir_list_head(&state->stack);
        iter != NULL;
        iter = kefir_list_head(&state->stack)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, key, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(mem, &state->stack, iter));
        kefir_opt_block_id_t block_id = (kefir_uint32_t) key;
        kefir_bool_t children_processed = (key >> 32) != 0;

        if (!children_processed) {
            key = (1ull << 32) | (kefir_uint32_t) block_id;
            REQUIRE_OK(kefir_list_insert_after(mem, &state->stack, NULL, (void *) (kefir_uptr_t) key));

            struct kefir_opt_control_flow_dominator_tree_iterator dom_iter;
            kefir_opt_block_id_t dominated_block_ref;
            kefir_result_t res;
            for (res = kefir_opt_control_flow_dominator_tree_iter(&state->control_flow, &dom_iter, block_id,
                                                                  &dominated_block_ref);
                 res == KEFIR_OK; res = kefir_opt_control_flow_dominator_tree_next(&dom_iter, &dominated_block_ref)) {
                REQUIRE_OK(kefir_list_insert_after(mem, &state->stack, NULL, (void *) (kefir_uptr_t) dominated_block_ref));
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        } else if (block_id != func->code.gate_block) {
            kefir_opt_instruction_ref_t block_tail_ref;
            const struct kefir_opt_instruction *block_tail;
            REQUIRE_OK(
                kefir_opt_code_block_instr_control_tail(&func->code, block_id, &block_tail_ref));
            if (block_tail_ref == KEFIR_ID_NONE) {
                continue;
            }
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, block_tail_ref, &block_tail));
            if (!(block_tail->operation.opcode == KEFIR_OPT_OPCODE_JUMP ||
                block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH ||
                block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH_COMPARE)) {
                continue;
            }

            kefir_hashset_key_t hash_key;
            struct kefir_hashset_iterator iter;
            kefir_result_t res;
            for (res = kefir_hashset_iter(&state->control_flow.blocks[block_id].successors, &iter, &hash_key);
                res == KEFIR_OK;
                res = kefir_hashset_next(&iter, &hash_key)) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, hash_key);

                kefir_opt_instruction_ref_t phi_instr_ref;
                REQUIRE_OK(kefir_opt_code_block_phi_head(&func->code, successor_block_id, &phi_instr_ref));

                kefir_opt_instruction_ref_t successor_block_tail_ref;
                REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, successor_block_id, &successor_block_tail_ref));

                if (successor_block_tail_ref == KEFIR_ID_NONE) {
                    continue;
                }

                const struct kefir_opt_instruction *successor_block_tail;
                REQUIRE_OK(kefir_opt_code_container_instr(&func->code, successor_block_tail_ref, &successor_block_tail));

                if (phi_instr_ref == KEFIR_ID_NONE && successor_block_id != func->code.entry_point && successor_block_id != func->code.gate_block &&
                    state->control_flow.blocks[successor_block_id].predecessors.occupied == 1 &&
                    kefir_hashset_has(&state->control_flow.blocks[successor_block_id].predecessors, (kefir_hashset_key_t) block_id) &&
                    !kefir_hashset_has(&state->control_flow.blocks[successor_block_id].successors, (kefir_hashset_key_t) func->code.gate_block) &&
                    !kefir_hashset_has(&state->control_flow.indirect_jump_target_blocks, (kefir_hashset_key_t) successor_block_id) &&
                    (successor_block_tail->operation.opcode == KEFIR_OPT_OPCODE_JUMP ||
                    successor_block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH ||
                    successor_block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH_COMPARE ||
                    successor_block_tail->operation.opcode == KEFIR_OPT_OPCODE_INLINE_ASSEMBLY ||
                    successor_block_tail->operation.opcode == KEFIR_OPT_OPCODE_RETURN ||
                    successor_block_tail->operation.opcode == KEFIR_OPT_OPCODE_UNREACHABLE ||
                    successor_block_tail->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE ||
                    successor_block_tail->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL)) {
                    key = (((kefir_uint64_t) successor_block_id) << 32) | (kefir_uint32_t) block_id;
                    REQUIRE_OK(kefir_list_insert_after(mem, &state->merge_order, kefir_list_tail(&state->merge_order), (void *) (kefir_uptr_t) key));
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t do_merge(struct kefir_mem *mem, struct kefir_opt_function *func,
                               kefir_opt_block_id_t block_id, kefir_opt_block_id_t successor_block_id,
                               kefir_bool_t *merged) {
    const struct kefir_opt_code_block *block, *successor_block;
    REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));
    REQUIRE_OK(kefir_opt_code_container_block(&func->code, successor_block_id, &successor_block));

    kefir_opt_instruction_ref_t block_tail_ref, successor_block_tail_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, block_id, &block_tail_ref));
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, successor_block_id, &successor_block_tail_ref));

    const struct kefir_opt_instruction *block_tail, *successor_block_tail;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, block_tail_ref, &block_tail));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, successor_block_tail_ref, &successor_block_tail));

    if (block_tail->operation.opcode != KEFIR_OPT_OPCODE_JUMP) {
        REQUIRE(successor_block_tail->operation.opcode == KEFIR_OPT_OPCODE_JUMP, KEFIR_OK);

        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_head(&func->code, successor_block_id, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_sibling(&func->code, instr_ref, &instr_ref)) {
            if (instr_ref == successor_block_tail_ref) {
                continue;
            }
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_ref, &instr));
            kefir_bool_t moveable;
            REQUIRE_OK(kefir_opt_instruction_is_moveable(&func->code, instr_ref, &moveable));
            REQUIRE(moveable, KEFIR_OK);
        }
        REQUIRE_OK(res);

        struct kefir_opt_operation oper = block_tail->operation;
        if (oper.parameters.branch.target_block == successor_block_id) {
            oper.parameters.branch.target_block = successor_block_tail->operation.parameters.branch.target_block;
        }
        if (oper.parameters.branch.alternative_block == successor_block_id) {
            oper.parameters.branch.alternative_block = successor_block_tail->operation.parameters.branch.target_block;
        }

        if (oper.parameters.branch.target_block == oper.parameters.branch.alternative_block) {
            const struct kefir_opt_code_block *target_block;
            REQUIRE_OK(kefir_opt_code_container_block(&func->code, oper.parameters.branch.target_block, &target_block));
            kefir_opt_instruction_ref_t phi_instr_ref;
            REQUIRE_OK(kefir_opt_code_block_phi_head(&func->code, oper.parameters.branch.target_block, &phi_instr_ref));
            REQUIRE(phi_instr_ref == KEFIR_ID_NONE, KEFIR_OK);
        }

        REQUIRE_OK(kefir_opt_code_block_redirect_phi_links(
            mem, &func->code, successor_block_id, block_id,
            (kefir_opt_block_id_t) successor_block_tail->operation.parameters.branch.target_block));

        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, block_tail_ref));
        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, block_tail_ref));
        REQUIRE_OK(kefir_opt_code_block_merge_into(mem, &func->code, &func->debug_info, block_id, successor_block_id,
                                                   false, true));

        kefir_opt_instruction_ref_t new_tail_instr_ref;
        if (oper.parameters.branch.target_block != oper.parameters.branch.alternative_block) {
            REQUIRE_OK(
                kefir_opt_code_container_new_instruction(mem, &func->code, block_id, &oper, &new_tail_instr_ref));
        } else {
            REQUIRE_OK(kefir_opt_code_container_new_instruction(
                mem, &func->code, block_id,
                &(struct kefir_opt_operation) {
                    .opcode = KEFIR_OPT_OPCODE_JUMP,
                    .parameters.branch = {.target_block = oper.parameters.branch.target_block,
                                          .alternative_block = KEFIR_ID_NONE,
                                          .condition_ref = KEFIR_ID_NONE}},
                &new_tail_instr_ref));
        }
        REQUIRE_OK(kefir_opt_code_container_add_control(&func->code, block_id, new_tail_instr_ref));

        *merged = true;
    } else {
        switch (successor_block_tail->operation.opcode) {
            case KEFIR_OPT_OPCODE_JUMP:
                REQUIRE_OK(kefir_opt_code_block_redirect_phi_links(
                    mem, &func->code, successor_block_id, block_id,
                    (kefir_opt_block_id_t) successor_block_tail->operation.parameters.branch.target_block));
                break;

            case KEFIR_OPT_OPCODE_BRANCH:
            case KEFIR_OPT_OPCODE_BRANCH_COMPARE:
                REQUIRE_OK(kefir_opt_code_block_redirect_phi_links(
                    mem, &func->code, successor_block_id, block_id,
                    (kefir_opt_block_id_t) successor_block_tail->operation.parameters.branch.target_block));
                REQUIRE_OK(kefir_opt_code_block_redirect_phi_links(
                    mem, &func->code, successor_block_id, block_id,
                    (kefir_opt_block_id_t) successor_block_tail->operation.parameters.branch.alternative_block));
                break;

            case KEFIR_OPT_OPCODE_INLINE_ASSEMBLY: {
                const struct kefir_opt_inline_assembly_node *inline_asm;
                REQUIRE_OK(kefir_opt_code_container_inline_assembly(
                    &func->code, successor_block_tail->operation.parameters.inline_asm_ref, &inline_asm));

                struct kefir_hashtree_node_iterator iter;
                for (const struct kefir_hashtree_node *node = kefir_hashtree_iter(&inline_asm->jump_targets, &iter);
                     node != NULL; node = kefir_hashtree_next(&iter)) {
                    ASSIGN_DECL_CAST(kefir_opt_block_id_t, target_block, node->value);
                    REQUIRE_OK(kefir_opt_code_block_redirect_phi_links(mem, &func->code, successor_block_id, block_id,
                                                                       (kefir_opt_block_id_t) target_block));
                }
            } break;

            case KEFIR_OPT_OPCODE_RETURN:
            case KEFIR_OPT_OPCODE_UNREACHABLE:
            case KEFIR_OPT_OPCODE_TAIL_INVOKE:
            case KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL:
                // Intentionally left blank
                break;

            default:
                return KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected successor block tail instruction");
        }

        REQUIRE_OK(kefir_opt_code_container_drop_control(&func->code, block_tail_ref));
        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, &func->code, block_tail_ref));

        REQUIRE_OK(kefir_opt_code_block_merge_into(mem, &func->code, &func->debug_info, block_id, successor_block_id,
                                                   true, true));
        *merged = true;
    }

    return KEFIR_OK;
}

static kefir_result_t block_merge_impl(struct kefir_mem *mem, struct kefir_opt_function *func,
                                       struct merge_state *state) {
    state->block_merges = KEFIR_MALLOC(mem, sizeof(kefir_uint8_t) * kefir_opt_code_container_block_count(&func->code));
    REQUIRE(state->block_merges != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate optimizer block merge counters"));
    memset(state->block_merges, 0, sizeof(kefir_uint8_t) * kefir_opt_code_container_block_count(&func->code));

    kefir_bool_t merged_blocks = true;
    for (; merged_blocks;) {
        REQUIRE_OK(kefir_opt_code_control_flow_build(mem, &state->control_flow, &func->code));
        REQUIRE_OK(collect_merge_order(mem, func, state));

        merged_blocks = false;
        for (const struct kefir_list_entry *iter = kefir_list_head(&state->merge_order);
            iter != NULL;
            kefir_list_next(&iter)) {
            ASSIGN_DECL_CAST(kefir_uint64_t, key, (kefir_uptr_t) iter->value);
            kefir_opt_block_id_t block_id = (kefir_uint32_t) key, successor_block_id = key >> 32;
            if (state->block_merges[block_id] + state->block_merges[successor_block_id] + 1 > MAX_MERGE_CHAIN) {
                continue;
            }
            REQUIRE_OK(do_merge(mem, func, block_id, successor_block_id, &merged_blocks));
            state->block_merges[block_id] += state->block_merges[successor_block_id] + 1;
        }
        REQUIRE_OK(kefir_list_clear(mem, &state->merge_order));
    }
    return KEFIR_OK;
}

static kefir_result_t merge_blocks_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                         struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                         const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct merge_state state = {
        .block_merges = NULL
    };
    REQUIRE_OK(kefir_opt_code_control_flow_init(&state.control_flow));
    REQUIRE_OK(kefir_list_init(&state.merge_order));
    REQUIRE_OK(kefir_list_init(&state.stack));
    kefir_result_t res = block_merge_impl(mem, func, &state);
    KEFIR_FREE(mem, state.block_merges);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.stack);
        kefir_list_free(mem, &state.merge_order);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.stack);
        kefir_list_free(mem, &state.merge_order);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_list_free(mem, &state.stack);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.merge_order);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_list_free(mem, &state.merge_order);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_control_flow_free(mem, &state.control_flow));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassMergeBlocks = {
    .name = "merge-blocks", .apply = merge_blocks_apply, .payload = NULL};

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
#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

struct merge_entry {
    struct kefir_hashset candidates;
};

struct merge_state {
    struct kefir_opt_code_control_flow control_flow;
    struct kefir_hashtable entries;
};

static kefir_result_t collect_candidates(struct kefir_mem *mem, struct kefir_opt_function *func,
                                         struct merge_state *state) {
    kefir_result_t res;
    kefir_size_t block_count;
    REQUIRE_OK(kefir_opt_code_container_block_count(&func->code, &block_count));

    for (kefir_opt_block_id_t block_ref = 0; block_ref < block_count; block_ref++) {
        kefir_opt_block_id_t immediate_dominator_ref = state->control_flow.blocks[block_ref].immediate_dominator;

        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_ref, &block));
        kefir_opt_phi_id_t phi_ref;
        REQUIRE_OK(kefir_opt_code_block_phi_head(&func->code, block, &phi_ref));
        if (block_ref != func->code.gate_block && immediate_dominator_ref != KEFIR_ID_NONE &&
            immediate_dominator_ref != func->code.gate_block &&
            kefir_hashset_size(&state->control_flow.blocks[block_ref].predecessors) == 1 &&
            kefir_hashset_has(&state->control_flow.blocks[block_ref].predecessors,
                              (kefir_hashset_key_t) immediate_dominator_ref) &&
            !kefir_hashset_has(&state->control_flow.indirect_jump_target_blocks, (kefir_hashset_key_t) block_ref) &&
            phi_ref == KEFIR_ID_NONE) {

            const struct kefir_opt_code_block *pred_block;
            REQUIRE_OK(kefir_opt_code_container_block(&func->code, immediate_dominator_ref, &pred_block));

            kefir_opt_instruction_ref_t pred_block_tail_ref, block_tail_ref;
            REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, pred_block, &pred_block_tail_ref));
            REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, block, &block_tail_ref));

            const struct kefir_opt_instruction *pred_block_tail, *block_tail;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, pred_block_tail_ref, &pred_block_tail));
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, block_tail_ref, &block_tail));

            if ((pred_block_tail->operation.opcode == KEFIR_OPT_OPCODE_JUMP ||
                 pred_block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH ||
                 pred_block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH_COMPARE) &&
                (block_tail->operation.opcode == KEFIR_OPT_OPCODE_JUMP ||
                 block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH ||
                 block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH_COMPARE ||
                 block_tail->operation.opcode == KEFIR_OPT_OPCODE_INLINE_ASSEMBLY ||
                 block_tail->operation.opcode == KEFIR_OPT_OPCODE_RETURN ||
                 block_tail->operation.opcode == KEFIR_OPT_OPCODE_UNREACHABLE ||
                 block_tail->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE ||
                 block_tail->operation.opcode == KEFIR_OPT_OPCODE_TAIL_INVOKE_VIRTUAL) &&
                !kefir_hashset_has(&state->control_flow.blocks[block_ref].successors,
                                   (kefir_hashset_key_t) func->code.gate_block)) {
                struct merge_entry *entry;
                kefir_hashtable_value_t table_value;
                res =
                    kefir_hashtable_at(&state->entries, (kefir_hashtable_key_t) immediate_dominator_ref, &table_value);
                if (res != KEFIR_NOT_FOUND) {
                    REQUIRE_OK(res);
                    entry = (struct merge_entry *) table_value;
                } else {
                    entry = KEFIR_MALLOC(mem, sizeof(struct merge_entry));
                    REQUIRE(entry != NULL,
                            KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate block merge entry"));
                    res = kefir_hashset_init(&entry->candidates, &kefir_hashtable_uint_ops);
                    REQUIRE_CHAIN(&res, kefir_hashtable_insert(mem, &state->entries,
                                                               (kefir_hashtable_key_t) immediate_dominator_ref,
                                                               (kefir_hashtable_value_t) entry));
                    REQUIRE_ELSE(res == KEFIR_OK, {
                        KEFIR_FREE(mem, entry);
                        return res;
                    });
                }
                REQUIRE_OK(kefir_hashset_add(mem, &entry->candidates, (kefir_hashset_key_t) block_ref));
            }
        }
    }
    return KEFIR_OK;
}

static kefir_result_t pick_candidate(struct kefir_mem *mem, struct kefir_opt_function *func, struct merge_state *state,
                                     kefir_opt_block_id_t *block_id_ptr, kefir_opt_block_id_t *successor_block_id_ptr) {
    UNUSED(func);
    kefir_result_t res;
    for (; state->entries.occupied > 0;) {
        struct kefir_hashtable_iterator iter;
        kefir_hashtable_key_t table_key;
        kefir_hashtable_value_t table_value;
        for (res = kefir_hashtable_iter(&state->entries, &iter, &table_key, &table_value); res == KEFIR_OK;
             res = kefir_hashtable_next(&iter, &table_key, &table_value)) {
            ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, table_key);
            ASSIGN_DECL_CAST(struct merge_entry *, entry, table_value);
            struct kefir_hashset_iterator candidate_iter;
            kefir_hashset_key_t cadidate_key;
            for (res = kefir_hashset_iter(&entry->candidates, &candidate_iter, &cadidate_key); res == KEFIR_OK;
                 res = kefir_hashset_next(&candidate_iter, &cadidate_key)) {
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, successor_block_id, cadidate_key);
                if (!kefir_hashtable_has(&state->entries, (kefir_hashtable_key_t) successor_block_id)) {
                    *block_id_ptr = block_id;
                    *successor_block_id_ptr = successor_block_id;
                    REQUIRE_OK(kefir_hashset_delete(&entry->candidates, (kefir_hashset_key_t) successor_block_id));
                    if (kefir_hashset_size(&entry->candidates) == 0) {
                        REQUIRE_OK(kefir_hashtable_delete(mem, &state->entries, (kefir_hashtable_key_t) block_id));
                    }
                    return KEFIR_OK;
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        REQUIRE_OK(kefir_hashtable_iter(&state->entries, &iter, &table_key, NULL));
        REQUIRE_OK(kefir_hashtable_delete(mem, &state->entries, table_key));
    }
    return KEFIR_SET_ERROR(KEFIR_NOT_FOUND, "Unable to find candidates for merge");
}

static kefir_result_t do_merge(struct kefir_mem *mem, struct kefir_opt_function *func, struct merge_state *state,
                               kefir_opt_block_id_t block_id, kefir_opt_block_id_t successor_block_id,
                               kefir_bool_t *merged) {
    UNUSED(state);
    const struct kefir_opt_code_block *block, *successor_block;
    REQUIRE_OK(kefir_opt_code_container_block(&func->code, block_id, &block));
    REQUIRE_OK(kefir_opt_code_container_block(&func->code, successor_block_id, &successor_block));

    kefir_opt_instruction_ref_t block_tail_ref, successor_block_tail_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, block, &block_tail_ref));
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&func->code, successor_block, &successor_block_tail_ref));

    const struct kefir_opt_instruction *block_tail, *successor_block_tail;
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, block_tail_ref, &block_tail));
    REQUIRE_OK(kefir_opt_code_container_instr(&func->code, successor_block_tail_ref, &successor_block_tail));

    if (block_tail->operation.opcode != KEFIR_OPT_OPCODE_JUMP) {
        REQUIRE(successor_block_tail->operation.opcode == KEFIR_OPT_OPCODE_JUMP, KEFIR_OK);

        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_head(&func->code, successor_block, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_sibling(&func->code, instr_ref, &instr_ref)) {
            if (instr_ref == successor_block_tail_ref) {
                continue;
            }
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&func->code, instr_ref, &instr));
            kefir_bool_t side_effect_free;
            REQUIRE_OK(kefir_opt_instruction_is_side_effect_free(instr, &side_effect_free));
            REQUIRE(side_effect_free, KEFIR_OK);
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
            kefir_opt_phi_id_t phi_ref;
            REQUIRE_OK(kefir_opt_code_block_phi_head(&func->code, target_block, &phi_ref));
            REQUIRE(phi_ref == KEFIR_ID_NONE, KEFIR_OK);
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

    kefir_bool_t merged_blocks = true;
    for (; merged_blocks;) {
        REQUIRE_OK(kefir_opt_code_control_flow_build(mem, &state->control_flow, &func->code));
        REQUIRE_OK(collect_candidates(mem, func, state));

        merged_blocks = false;
        for (;;) {
            kefir_opt_block_id_t block_id = KEFIR_ID_NONE, successor_block_id = KEFIR_ID_NONE;
            kefir_result_t res = pick_candidate(mem, func, state, &block_id, &successor_block_id);
            if (res == KEFIR_NOT_FOUND) {
                break;
            }
            REQUIRE_OK(res);
            REQUIRE_OK(do_merge(mem, func, state, block_id, successor_block_id, &merged_blocks));
        }
    }
    return KEFIR_OK;
}

kefir_result_t free_entry(struct kefir_mem *mem, struct kefir_hashtable *table, kefir_hashtable_key_t key,
                          kefir_hashtable_value_t value, void *payload) {
    UNUSED(table);
    UNUSED(key);
    UNUSED(payload);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    ASSIGN_DECL_CAST(struct merge_entry *, entry, value);
    REQUIRE(entry != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer block merge entry"));

    REQUIRE_OK(kefir_hashset_free(mem, &entry->candidates));
    KEFIR_FREE(mem, entry);
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

    struct merge_state state;
    REQUIRE_OK(kefir_opt_code_control_flow_init(&state.control_flow));
    REQUIRE_OK(kefir_hashtable_init(&state.entries, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_on_removal(&state.entries, free_entry, NULL));
    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, block_merge_impl(mem, func, &state));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.entries);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.entries);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_control_flow_free(mem, &state.control_flow));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassMergeBlocks = {
    .name = "merge-blocks", .apply = merge_blocks_apply, .payload = NULL};

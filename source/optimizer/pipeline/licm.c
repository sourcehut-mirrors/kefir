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
#include "kefir/optimizer/loop_nest.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>
#include <stdio.h>

struct licm_state {
    struct kefir_mem *mem;
    struct kefir_opt_function *func;
    struct kefir_opt_code_control_flow control_flow;
    struct kefir_opt_code_loop_collection loops;

    const struct kefir_opt_code_loop *loop;
    struct kefir_hashset candidate_blocks;

    kefir_bool_t all_inputs_nonlocal;
    kefir_bool_t extract_candidate_inputs;

    struct kefir_list candidate_queue;
    struct kefir_hashset candidate_queue_index;
    struct kefir_hashset rejected_candidates;

    kefir_opt_instruction_ref_t phi_input;
    struct kefir_hashset skip_phi_uses;
};

static kefir_result_t all_inputs_processed(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct licm_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid LICM state"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));

    const kefir_bool_t loop_local =
        kefir_hashtreeset_has(&state->loop->loop_blocks, (kefir_hashtreeset_entry_t) instr->block_id);
    if (state->extract_candidate_inputs && loop_local &&
        !kefir_hashset_has(&state->rejected_candidates, (kefir_hashset_key_t) instr_ref)) {
        if (!kefir_hashset_has(&state->candidate_queue_index, (kefir_hashset_key_t) instr_ref)) {
            kefir_uint64_t key = (kefir_uint32_t) instr_ref;
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->candidate_queue, NULL, (void *) (kefir_uptr_t) key));
            REQUIRE_OK(
                kefir_hashset_add(state->mem, &state->candidate_queue_index, (kefir_hashtreeset_entry_t) instr_ref));
        }
    }

    state->all_inputs_nonlocal = state->all_inputs_nonlocal && !loop_local;
    return KEFIR_OK;
}

#define IS_BLOCK_REACHABLE(_control_flow, _block_id)      \
    ((_block_id) == (_control_flow)->code->entry_point || \
     (_control_flow)->blocks[(_block_id)].immediate_dominator != KEFIR_ID_NONE)

static kefir_result_t update_nest(struct kefir_mem *mem, const struct kefir_tree_node *nest,
                                  kefir_opt_block_id_t loop_entry_id, kefir_opt_block_id_t preheader_id) {
    struct kefir_opt_code_loop *loop = nest->value;
    if (loop->loop_entry_block_id != loop_entry_id &&
        kefir_hashtreeset_has(&loop->loop_blocks, (kefir_hashtreeset_entry_t) loop_entry_id)) {
        REQUIRE_OK(kefir_hashtreeset_add(mem, &loop->loop_blocks, (kefir_hashtreeset_entry_t) preheader_id));
    }

    for (struct kefir_tree_node *child = kefir_tree_first_child(nest); child != NULL;
         child = kefir_tree_next_sibling(child)) {
        REQUIRE_OK(update_nest(mem, child, loop_entry_id, preheader_id));
    }
    return KEFIR_OK;
}

static kefir_result_t insert_predecessor_block_impl(
    struct kefir_mem *mem, struct kefir_opt_code_control_flow *control_flow, struct kefir_opt_code_container *code,
    struct kefir_opt_code_loop_collection *loops, kefir_opt_block_id_t loop_entry, kefir_opt_block_id_t loop_exit,
    struct kefir_list *phi_queue, kefir_opt_block_id_t *predecessor_block_id_ptr) {
    kefir_opt_block_id_t predecessor_block_id;
    REQUIRE_OK(kefir_opt_code_container_new_block(mem, code, false, &predecessor_block_id));
    REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, code, predecessor_block_id, loop_entry, NULL));

    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t entry;
    for (res = kefir_hashset_iter(&control_flow->blocks[loop_entry].predecessors, &iter, &entry); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &entry)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, current_pred_block_id, entry);
        if (current_pred_block_id != loop_exit && IS_BLOCK_REACHABLE(control_flow, current_pred_block_id)) {
            const struct kefir_opt_code_block *block;
            REQUIRE_OK(kefir_opt_code_container_block(code, current_pred_block_id, &block));

            kefir_opt_instruction_ref_t control_tail_ref;
            REQUIRE_OK(kefir_opt_code_block_instr_control_tail(code, current_pred_block_id, &control_tail_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_replace_control_flow_target(
                code, control_tail_ref, loop_entry, predecessor_block_id));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_opt_instruction_ref_t phi_instr_ref;
    const struct kefir_opt_code_block *loop_entry_block;
    REQUIRE_OK(kefir_opt_code_container_block(code, loop_entry, &loop_entry_block));
    for (res = kefir_opt_code_block_phi_head(code, loop_entry, &phi_instr_ref);
         res == KEFIR_OK && phi_instr_ref != KEFIR_ID_NONE;
         kefir_opt_phi_next_sibling(code, phi_instr_ref, &phi_instr_ref)) {
        REQUIRE_OK(
            kefir_list_insert_after(mem, phi_queue, kefir_list_tail(phi_queue), (void *) (kefir_uptr_t) phi_instr_ref));
    }
    REQUIRE_OK(res);

    for (const struct kefir_list_entry *phi_iter = kefir_list_head(phi_queue); phi_iter != NULL;
         kefir_list_next(&phi_iter)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, phi_instr_ref, (kefir_uptr_t) phi_iter->value);
        kefir_opt_instruction_ref_t predecessor_phi_instr_ref, replacement_phi_instr_ref;
        REQUIRE_OK(kefir_opt_code_container_new_phi(mem, code, predecessor_block_id, &predecessor_phi_instr_ref));
        REQUIRE_OK(kefir_opt_code_container_new_phi(mem, code, loop_entry, &replacement_phi_instr_ref));

        struct kefir_opt_phi_node_link_iterator link_iter;
        kefir_opt_block_id_t link_block_id;
        kefir_opt_instruction_ref_t link_instr_ref;
        for (res = kefir_opt_phi_node_link_iter(code, phi_instr_ref, &link_iter, &link_block_id, &link_instr_ref);
             res == KEFIR_OK; res = kefir_opt_phi_node_link_next(&link_iter, &link_block_id, &link_instr_ref)) {
            if (link_block_id == loop_exit) {
                REQUIRE_OK(kefir_opt_code_container_phi_attach(mem, code, replacement_phi_instr_ref, loop_exit,
                                                               link_instr_ref));
            } else {
                REQUIRE_OK(kefir_opt_code_container_phi_attach(mem, code, predecessor_phi_instr_ref, link_block_id,
                                                               link_instr_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        REQUIRE_OK(kefir_opt_code_container_phi_attach(mem, code, replacement_phi_instr_ref, predecessor_block_id,
                                                       predecessor_phi_instr_ref));
        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, code, replacement_phi_instr_ref, phi_instr_ref));
        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, code, phi_instr_ref));
    }

    REQUIRE_OK(kefir_opt_code_control_flow_free(mem, control_flow));
    REQUIRE_OK(kefir_opt_code_control_flow_init(control_flow));
    REQUIRE_OK(kefir_opt_code_control_flow_build(mem, control_flow, code));

    const struct kefir_opt_loop_nest *nest;
    struct kefir_opt_code_loop_nest_collection_iterator nest_iter;
    for (res = kefir_opt_code_loop_nest_collection_iter(loops, &nest, &nest_iter); res == KEFIR_OK && nest != NULL;
         res = kefir_opt_code_loop_nest_collection_next(&nest, &nest_iter)) {
        REQUIRE_OK(update_nest(mem, &nest->nest, loop_entry, predecessor_block_id));
    }

    *predecessor_block_id_ptr = predecessor_block_id;
    return KEFIR_OK;
}

static kefir_result_t insert_predecessor_block(struct kefir_mem *mem, struct kefir_opt_code_control_flow *control_flow,
                                               struct kefir_opt_code_container *code,
                                               struct kefir_opt_code_loop_collection *loops,
                                               kefir_opt_block_id_t loop_entry, kefir_opt_block_id_t loop_exit,
                                               kefir_opt_block_id_t *predecessor_block_id_ptr) {
    struct kefir_list phi_queue;
    REQUIRE_OK(kefir_list_init(&phi_queue));

    kefir_result_t res = insert_predecessor_block_impl(mem, control_flow, code, loops, loop_entry, loop_exit,
                                                       &phi_queue, predecessor_block_id_ptr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &phi_queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &phi_queue));
    return KEFIR_OK;
}

static kefir_result_t do_hoist(struct licm_state *state, kefir_opt_block_id_t loop_entry,
                               kefir_opt_block_id_t loop_exit, kefir_opt_instruction_ref_t instr_ref,
                               kefir_opt_block_id_t *hoist_target) {
    kefir_result_t res;
    const struct kefir_opt_instruction *instr;
    if (*hoist_target == KEFIR_ID_NONE) {
        REQUIRE_OK(insert_predecessor_block(state->mem, &state->control_flow, &state->func->code, &state->loops,
                                            loop_entry, loop_exit, hoist_target));
        res = kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr);
        REQUIRE(res != KEFIR_NOT_FOUND, KEFIR_OK);
        REQUIRE_OK(res);
    }
    kefir_bool_t can_hoist;
    REQUIRE_OK(kefir_opt_can_hoist_instruction(&state->control_flow, instr_ref, *hoist_target, &can_hoist));
    if (can_hoist) {
        REQUIRE_OK(kefir_opt_move_instruction(state->mem, &state->func->code, &state->func->debug_info, instr_ref,
                                              *hoist_target, &instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t all_inputs_phi_or_nonlocal(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct licm_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid LICM state"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));

    if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI && instr->block_id == state->loop->loop_entry_block_id &&
        state->phi_input == KEFIR_ID_NONE) {
        state->phi_input = instr->id;
        return KEFIR_OK;
    }

    const kefir_bool_t loop_local =
        kefir_hashtreeset_has(&state->loop->loop_blocks, (kefir_hashtreeset_entry_t) instr->block_id);
    if (loop_local) {
        state->all_inputs_nonlocal = false;
    }
    return KEFIR_OK;
}

static kefir_result_t distribute_over_phi(struct licm_state *state, kefir_opt_instruction_ref_t phi_instr_ref,
                                          kefir_opt_instruction_ref_t instr_ref, kefir_bool_t *fixpoint_reached) {
    const struct kefir_opt_instruction *phi_instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, phi_instr_ref, &phi_instr));

    kefir_opt_instruction_ref_t new_phi_instr_ref = KEFIR_ID_NONE;

    kefir_result_t res;
    struct kefir_opt_phi_node_link_iterator link_iter;
    kefir_opt_block_id_t link_block_id;
    kefir_opt_instruction_ref_t link_instr_ref;
    for (res = kefir_opt_phi_node_link_iter(&state->func->code, phi_instr_ref, &link_iter, &link_block_id,
                                            &link_instr_ref);
         res == KEFIR_OK; res = kefir_opt_phi_node_link_next(&link_iter, &link_block_id, &link_instr_ref)) {

        if (new_phi_instr_ref == KEFIR_ID_NONE) {
            REQUIRE_OK(kefir_opt_code_container_new_phi(state->mem, &state->func->code,
                                                        state->loop->loop_entry_block_id, &new_phi_instr_ref));
        }

        kefir_opt_instruction_ref_t copy_instr_ref;
        REQUIRE_OK(kefir_opt_code_container_copy_instruction(state->mem, &state->func->code, link_block_id, instr_ref,
                                                             &copy_instr_ref));
        REQUIRE_OK(kefir_opt_code_container_replace_references_in(state->mem, &state->func->code, copy_instr_ref,
                                                                  link_instr_ref, phi_instr_ref));
        REQUIRE_OK(kefir_opt_code_container_phi_attach(state->mem, &state->func->code, new_phi_instr_ref, link_block_id,
                                                       copy_instr_ref));
        REQUIRE_OK(kefir_hashset_add(state->mem, &state->skip_phi_uses, (kefir_hashset_key_t) copy_instr_ref));
        *fixpoint_reached = false;
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    if (new_phi_instr_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(
            kefir_opt_code_container_replace_references(state->mem, &state->func->code, new_phi_instr_ref, instr_ref));
        REQUIRE_OK(kefir_opt_code_container_drop_instr(state->mem, &state->func->code, instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t has_transitive_use(struct licm_state *state, kefir_size_t depth, kefir_opt_block_id_t block_ref,
                                         kefir_opt_instruction_ref_t instr_ref, kefir_opt_instruction_ref_t search_ref,
                                         kefir_bool_t *has_use) {
    REQUIRE(depth > 0, KEFIR_OK);

    struct kefir_opt_instruction_use_iterator use_iter;
    kefir_result_t res;
    for (res = kefir_opt_code_container_instruction_use_instr_iter(&state->func->code, instr_ref, &use_iter);
         res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {

        const struct kefir_opt_instruction *use_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, use_iter.use_instr_ref, &use_instr));

        if (use_iter.use_instr_ref == search_ref) {
            *has_use = true;
            return KEFIR_OK;
        } else if (use_instr->block_id != block_ref) {
            continue;
        }

        REQUIRE_OK(has_transitive_use(state, depth - 1, block_ref, use_iter.use_instr_ref, search_ref, has_use));
        REQUIRE(!*has_use, KEFIR_OK);
    }
    return KEFIR_OK;
}

static kefir_result_t distribute_condition_dependencies_over_phis(struct licm_state *state) {
    kefir_bool_t fixpoint_reached = false;

    kefir_opt_instruction_ref_t entry_tail_ref;
    REQUIRE_OK(
        kefir_opt_code_block_instr_control_tail(&state->func->code, state->loop->loop_entry_block_id, &entry_tail_ref));

    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->skip_phi_uses));
    for (; !fixpoint_reached;) {
        fixpoint_reached = true;

        kefir_result_t res;
        kefir_opt_instruction_ref_t phi_instr_ref;
        for (res = kefir_opt_code_block_phi_head(&state->func->code, state->loop->loop_entry_block_id, &phi_instr_ref);
             res == KEFIR_OK && phi_instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_phi_next_sibling(&state->func->code, phi_instr_ref, &phi_instr_ref)) {

            struct kefir_opt_instruction_use_iterator use_iter;
            kefir_result_t res;
            for (res =
                     kefir_opt_code_container_instruction_use_instr_iter(&state->func->code, phi_instr_ref, &use_iter);
                 res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
                if (kefir_hashset_has(&state->skip_phi_uses, (kefir_hashset_key_t) use_iter.use_instr_ref)) {
                    continue;
                }
                const struct kefir_opt_instruction *use_instr;
                REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, use_iter.use_instr_ref, &use_instr));

                if (use_instr->block_id != state->loop->loop_entry_block_id ||
                    use_instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
                    continue;
                }

                kefir_bool_t side_effect_free, control_flow;
                REQUIRE_OK(kefir_opt_instruction_is_side_effect_free(use_instr, &side_effect_free));
                REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&state->func->code, use_iter.use_instr_ref,
                                                                      &control_flow));
                if (!side_effect_free || control_flow) {
                    continue;
                }

                kefir_bool_t transitive_uses = false;
                REQUIRE_OK(has_transitive_use(state, 32, state->loop->loop_entry_block_id, use_iter.use_instr_ref,
                                              entry_tail_ref, &transitive_uses));
                if (!transitive_uses) {
                    continue;
                }

                state->all_inputs_nonlocal = true;
                state->phi_input = KEFIR_ID_NONE;
                REQUIRE_OK(kefir_opt_instruction_extract_inputs(&state->func->code, use_instr, false,
                                                                all_inputs_phi_or_nonlocal, state));

                if (state->all_inputs_nonlocal && state->phi_input != KEFIR_ID_NONE) {
                    REQUIRE_OK(distribute_over_phi(state, state->phi_input, use_iter.use_instr_ref, &fixpoint_reached));
                    break;
                }
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

static kefir_result_t process_loop(struct licm_state *state) {
    REQUIRE(state->loop->loop_entry_block_id != state->control_flow.code->entry_point &&
                state->loop->loop_entry_block_id != state->control_flow.code->gate_block &&
                !kefir_hashset_has(&state->control_flow.indirect_jump_target_blocks,
                                   (kefir_hashset_key_t) state->loop->loop_entry_block_id),
            KEFIR_OK);

    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->candidate_blocks));
    REQUIRE_OK(kefir_list_clear(state->mem, &state->candidate_queue));
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->candidate_queue_index));
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->rejected_candidates));

    kefir_result_t res;
    struct kefir_hashset_iterator terminal_block_iter;
    kefir_hashset_key_t terminal_block_key;
    kefir_opt_block_id_t terminal_blocks_dominator = KEFIR_ID_NONE;
    for (res = kefir_hashset_iter(&state->loop->loop_exits_or_backedges, &terminal_block_iter, &terminal_block_key);
         res == KEFIR_OK; res = kefir_hashset_next(&terminal_block_iter, &terminal_block_key)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, terminal_block_key);
        if (block_id == state->loop->loop_entry_block_id) {
            continue;
        }
        kefir_bool_t is_reachable;
        REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(&state->control_flow, block_id, &is_reachable));
        if (!is_reachable) {
            continue;
        }

        if (terminal_blocks_dominator == KEFIR_ID_NONE) {
            terminal_blocks_dominator = block_id;
        } else {
            REQUIRE_OK(kefir_opt_find_closest_common_dominator(&state->control_flow, block_id,
                                                               terminal_blocks_dominator, &terminal_blocks_dominator));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    if (terminal_blocks_dominator == KEFIR_ID_NONE) {
        terminal_blocks_dominator = state->loop->loop_entry_block_id;
    }
    kefir_bool_t terminal_blocks_dominator_valid;
    REQUIRE_OK(kefir_opt_code_control_flow_is_dominator(&state->control_flow, terminal_blocks_dominator,
                                                        state->loop->loop_entry_block_id,
                                                        &terminal_blocks_dominator_valid));
    REQUIRE(terminal_blocks_dominator_valid, KEFIR_OK);

    REQUIRE_OK(
        kefir_hashset_add(state->mem, &state->candidate_blocks, (kefir_hashset_key_t) terminal_blocks_dominator));
    for (; terminal_blocks_dominator != state->loop->loop_entry_block_id;) {
        terminal_blocks_dominator = state->control_flow.blocks[terminal_blocks_dominator].immediate_dominator;
        REQUIRE_OK(
            kefir_hashset_add(state->mem, &state->candidate_blocks, (kefir_hashset_key_t) terminal_blocks_dominator));
    }

    struct kefir_hashtreeset_iterator iter;
    for (res = kefir_hashtreeset_iter(&state->loop->loop_blocks, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter.entry);
        kefir_bool_t is_reachable;
        REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(&state->control_flow, block_id, &is_reachable));
        if (!is_reachable) {
            continue;
        }

        kefir_bool_t dominated_by_entry;
        REQUIRE_OK(kefir_opt_code_control_flow_is_dominator(&state->control_flow, block_id,
                                                            state->loop->loop_entry_block_id, &dominated_by_entry));
        REQUIRE(dominated_by_entry, KEFIR_OK);

        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&state->func->code, block_id, &block));

        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_control_head(&state->func->code, block_id, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_control(&state->func->code, instr_ref, &instr_ref)) {
            kefir_uint64_t key = (kefir_uint32_t) instr_ref;
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->candidate_queue,
                                               kefir_list_tail(&state->candidate_queue), (void *) (kefir_uptr_t) key));
            REQUIRE_OK(
                kefir_hashset_add(state->mem, &state->candidate_queue_index, (kefir_hashtreeset_entry_t) instr_ref));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_opt_block_id_t hoist_target = KEFIR_ID_NONE;
    for (struct kefir_list_entry *iter = kefir_list_head(&state->candidate_queue); iter != NULL;
         iter = kefir_list_head(&state->candidate_queue)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, candidate_key, (kefir_uptr_t) iter->value);
        kefir_bool_t candidate_inputs_pending = !(candidate_key >> 32);
        kefir_opt_instruction_ref_t instr_ref = (kefir_uint32_t) candidate_key;
        REQUIRE_OK(kefir_list_pop(state->mem, &state->candidate_queue, iter));
        REQUIRE_OK(kefir_hashset_delete(&state->candidate_queue_index, (kefir_hashset_key_t) instr_ref));
        if (kefir_hashset_has(&state->rejected_candidates, (kefir_hashset_key_t) instr_ref)) {
            continue;
        }

        const struct kefir_opt_instruction *instr;
        res = kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
            continue;
        }

        const struct kefir_list_entry *queue_head = kefir_list_head(&state->candidate_queue);
        state->all_inputs_nonlocal = true;
        state->extract_candidate_inputs = candidate_inputs_pending;
        REQUIRE_OK(kefir_opt_instruction_extract_inputs(&state->func->code, instr, false, all_inputs_processed, state));

        if (!state->all_inputs_nonlocal) {
            if (candidate_inputs_pending) {
                struct kefir_list_entry *insert_pos =
                    queue_head != NULL ? queue_head->prev : kefir_list_tail(&state->candidate_queue);
                kefir_uint64_t key = (1ull << 63) | (kefir_uint32_t) instr_ref;
                REQUIRE_OK(kefir_list_insert_after(state->mem, &state->candidate_queue, insert_pos,
                                                   (void *) (kefir_uptr_t) key));
                REQUIRE_OK(kefir_hashset_add(state->mem, &state->candidate_queue_index,
                                             (kefir_hashtreeset_entry_t) instr_ref));
            } else {
                REQUIRE_OK(kefir_hashset_add(state->mem, &state->rejected_candidates, (kefir_hashset_key_t) instr_ref));
            }
            continue;
        }

        kefir_bool_t is_side_effect_free, is_control_flow;
        REQUIRE_OK(kefir_opt_instruction_is_side_effect_free(instr, &is_side_effect_free));
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(state->control_flow.code, instr_ref, &is_control_flow));
        if (is_side_effect_free && !is_control_flow &&
            kefir_hashset_has(&state->candidate_blocks, (kefir_hashset_key_t) instr->block_id)) {
            REQUIRE_OK(do_hoist(state, state->loop->loop_entry_block_id, state->loop->loop_exit_block_id, instr_ref,
                                &hoist_target));
        }
    }

    REQUIRE_OK(distribute_condition_dependencies_over_phis(state));
    return KEFIR_OK;
}

static kefir_result_t process_nest(struct licm_state *state, const struct kefir_tree_node *nest) {
    for (struct kefir_tree_node *child = kefir_tree_first_child(nest); child != NULL;
         child = kefir_tree_next_sibling(child)) {
        REQUIRE_OK(process_nest(state, child));
    }

    state->loop = nest->value;
    REQUIRE_OK(process_loop(state));
    return KEFIR_OK;
}

static kefir_result_t licm_impl(struct licm_state *state) {
    REQUIRE_OK(kefir_opt_code_control_flow_build(state->mem, &state->control_flow, &state->func->code));
    REQUIRE_OK(kefir_opt_code_loop_collection_build(state->mem, &state->loops, &state->control_flow));

    kefir_result_t res;
    const struct kefir_opt_loop_nest *nest;
    struct kefir_opt_code_loop_nest_collection_iterator iter;
    for (res = kefir_opt_code_loop_nest_collection_iter(&state->loops, &nest, &iter); res == KEFIR_OK && nest != NULL;
         res = kefir_opt_code_loop_nest_collection_next(&nest, &iter)) {
        REQUIRE_OK(process_nest(state, &nest->nest));
    }

    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t loop_invariant_code_motion_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                                       struct kefir_opt_function *func,
                                                       const struct kefir_optimizer_pass *pass,
                                                       const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct licm_state state = {.mem = mem, .func = func};
    REQUIRE_OK(kefir_hashset_init(&state.candidate_queue_index, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&state.rejected_candidates, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_list_init(&state.candidate_queue));
    REQUIRE_OK(kefir_hashset_init(&state.skip_phi_uses, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&state.candidate_blocks, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_opt_code_control_flow_init(&state.control_flow));
    REQUIRE_OK(kefir_opt_code_loop_collection_init(&state.loops));

    kefir_result_t res = licm_impl(&state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_loop_collection_free(mem, &state.loops);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        kefir_hashset_free(mem, &state.candidate_blocks);
        kefir_hashset_free(mem, &state.skip_phi_uses);
        kefir_list_free(mem, &state.candidate_queue);
        kefir_hashset_free(mem, &state.candidate_queue_index);
        kefir_hashset_free(mem, &state.rejected_candidates);
        return res;
    });
    res = kefir_opt_code_loop_collection_free(mem, &state.loops);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        kefir_hashset_free(mem, &state.candidate_blocks);
        kefir_hashset_free(mem, &state.skip_phi_uses);
        kefir_list_free(mem, &state.candidate_queue);
        kefir_hashset_free(mem, &state.candidate_queue_index);
        kefir_hashset_free(mem, &state.rejected_candidates);
        return res;
    });
    res = kefir_opt_code_control_flow_free(mem, &state.control_flow);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.candidate_blocks);
        kefir_hashset_free(mem, &state.skip_phi_uses);
        kefir_list_free(mem, &state.candidate_queue);
        kefir_hashset_free(mem, &state.candidate_queue_index);
        kefir_hashset_free(mem, &state.rejected_candidates);
        return res;
    });
    res = kefir_hashset_free(mem, &state.candidate_blocks);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.skip_phi_uses);
        kefir_list_free(mem, &state.candidate_queue);
        kefir_hashset_free(mem, &state.candidate_queue_index);
        kefir_hashset_free(mem, &state.rejected_candidates);
        return res;
    });
    res = kefir_hashset_free(mem, &state.skip_phi_uses);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.candidate_queue);
        kefir_hashset_free(mem, &state.candidate_queue_index);
        kefir_hashset_free(mem, &state.rejected_candidates);
        return res;
    });
    res = kefir_list_free(mem, &state.candidate_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.candidate_queue_index);
        kefir_hashset_free(mem, &state.rejected_candidates);
        return res;
    });
    res = kefir_hashset_free(mem, &state.candidate_queue_index);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.rejected_candidates);
        return res;
    });
    REQUIRE_OK(kefir_hashset_free(mem, &state.rejected_candidates));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassLoopInvariantCodeMotion = {
    .name = "licm", .apply = loop_invariant_code_motion_apply, .payload = NULL};

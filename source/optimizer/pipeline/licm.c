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
#include "kefir/optimizer/loop_nest.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>
#include <stdio.h>

struct licm_state {
    struct kefir_mem *mem;
    struct kefir_opt_function *func;
    struct kefir_opt_code_structure structure;
    struct kefir_opt_code_loop_collection loops;

    const struct kefir_opt_code_loop *loop;

    struct kefir_hashtreeset processed_instr;
    struct kefir_hashtreeset hoist_candidates;
    struct kefir_hashtreeset traversal_queue_index;
    struct kefir_list traversal_queue;
    kefir_bool_t all_inputs_processed;
    kefir_bool_t all_inputs_nonlocal;
};

static kefir_result_t all_inputs_processed(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct licm_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid LICM state"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));

    const kefir_bool_t loop_local =
        kefir_hashtreeset_has(&state->loop->loop_blocks, (kefir_hashtreeset_entry_t) instr->block_id);
    if (!kefir_hashtreeset_has(&state->processed_instr, (kefir_hashtreeset_entry_t) instr_ref) && loop_local) {
        if (!kefir_hashtreeset_has(&state->traversal_queue_index, (kefir_hashtreeset_entry_t) instr_ref)) {
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->traversal_queue,
                                               kefir_list_tail(&state->traversal_queue),
                                               (void *) (kefir_uptr_t) instr_ref));
            REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->traversal_queue_index,
                                             (kefir_hashtreeset_entry_t) instr_ref));
        }
        state->all_inputs_processed = false;
    }

    state->all_inputs_nonlocal =
        state->all_inputs_nonlocal &&
        (!loop_local || kefir_hashtreeset_has(&state->hoist_candidates, (kefir_hashtreeset_entry_t) instr_ref));
    return KEFIR_OK;
}

#define IS_BLOCK_REACHABLE(_structure, _block_id)      \
    ((_block_id) == (_structure)->code->entry_point || \
     (_structure)->blocks[(_block_id)].immediate_dominator != KEFIR_ID_NONE)

static kefir_result_t insert_predecessor_block_impl(struct kefir_mem *mem, struct kefir_opt_code_structure *structure,
                                                    struct kefir_opt_code_container *code,
                                                    kefir_opt_block_id_t loop_entry, kefir_opt_block_id_t loop_exit,
                                                    struct kefir_list *phi_queue,
                                                    kefir_opt_block_id_t *predecessor_block_id_ptr) {
    kefir_opt_block_id_t predecessor_block_id;
    REQUIRE_OK(kefir_opt_code_container_new_block(mem, code, false, &predecessor_block_id));
    REQUIRE_OK(kefir_opt_code_builder_finalize_jump(mem, code, predecessor_block_id, loop_entry, NULL));

    for (const struct kefir_list_entry *iter = kefir_list_head(&structure->blocks[loop_entry].predecessors);
         iter != NULL; kefir_list_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, current_pred_block_id, (kefir_uptr_t) iter->value);
        if (current_pred_block_id != loop_exit && IS_BLOCK_REACHABLE(structure, current_pred_block_id)) {
            const struct kefir_opt_code_block *block;
            REQUIRE_OK(kefir_opt_code_container_block(code, current_pred_block_id, &block));

            kefir_opt_instruction_ref_t control_tail_ref;
            REQUIRE_OK(kefir_opt_code_block_instr_control_tail(code, block, &control_tail_ref));
            REQUIRE_OK(kefir_opt_code_container_instruction_replace_control_flow_target(
                code, control_tail_ref, loop_entry, predecessor_block_id));
        }
    }

    kefir_result_t res;
    kefir_opt_phi_id_t phi_ref;
    const struct kefir_opt_code_block *loop_entry_block;
    REQUIRE_OK(kefir_opt_code_container_block(code, loop_entry, &loop_entry_block));
    for (res = kefir_opt_code_block_phi_head(code, loop_entry_block, &phi_ref);
         res == KEFIR_OK && phi_ref != KEFIR_ID_NONE; kefir_opt_phi_next_sibling(code, phi_ref, &phi_ref)) {
        REQUIRE_OK(
            kefir_list_insert_after(mem, phi_queue, kefir_list_tail(phi_queue), (void *) (kefir_uptr_t) phi_ref));
    }
    REQUIRE_OK(res);

    for (const struct kefir_list_entry *phi_iter = kefir_list_head(phi_queue); phi_iter != NULL;
         kefir_list_next(&phi_iter)) {
        ASSIGN_DECL_CAST(kefir_opt_phi_id_t, phi_ref, (kefir_uptr_t) phi_iter->value);
        kefir_opt_phi_id_t predecessor_phi_ref, replacement_phi_ref;
        kefir_opt_instruction_ref_t predecessor_phi_instr_ref, replacement_phi_instr_ref;
        REQUIRE_OK(kefir_opt_code_container_new_phi(mem, code, predecessor_block_id, &predecessor_phi_ref,
                                                    &predecessor_phi_instr_ref));
        REQUIRE_OK(
            kefir_opt_code_container_new_phi(mem, code, loop_entry, &replacement_phi_ref, &replacement_phi_instr_ref));

        const struct kefir_opt_phi_node *phi_node;
        REQUIRE_OK(kefir_opt_code_container_phi(code, phi_ref, &phi_node));
        kefir_opt_instruction_ref_t phi_instr_ref = phi_node->output_ref;
        struct kefir_opt_phi_node_link_iterator link_iter;
        kefir_opt_block_id_t link_block_id;
        kefir_opt_instruction_ref_t link_instr_ref;
        for (res = kefir_opt_phi_node_link_iter(phi_node, &link_iter, &link_block_id, &link_instr_ref); res == KEFIR_OK;
             res = kefir_opt_phi_node_link_next(&link_iter, &link_block_id, &link_instr_ref)) {
            if (link_block_id == loop_exit) {
                REQUIRE_OK(
                    kefir_opt_code_container_phi_attach(mem, code, replacement_phi_ref, loop_exit, link_instr_ref));
            } else {
                REQUIRE_OK(
                    kefir_opt_code_container_phi_attach(mem, code, predecessor_phi_ref, link_block_id, link_instr_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        REQUIRE_OK(kefir_opt_code_container_phi_attach(mem, code, replacement_phi_ref, predecessor_block_id,
                                                       predecessor_phi_instr_ref));
        REQUIRE_OK(kefir_opt_code_container_replace_references(mem, code, replacement_phi_instr_ref, phi_instr_ref));
        REQUIRE_OK(kefir_opt_code_container_drop_instr(mem, code, phi_instr_ref));
    }

    REQUIRE_OK(kefir_opt_code_structure_free(mem, structure));
    REQUIRE_OK(kefir_opt_code_structure_init(structure));
    REQUIRE_OK(kefir_opt_code_structure_build(mem, structure, code));

    *predecessor_block_id_ptr = predecessor_block_id;
    return KEFIR_OK;
}

static kefir_result_t insert_predecessor_block(struct kefir_mem *mem, struct kefir_opt_code_structure *structure,
                                               struct kefir_opt_code_container *code, kefir_opt_block_id_t loop_entry,
                                               kefir_opt_block_id_t loop_exit,
                                               kefir_opt_block_id_t *predecessor_block_id_ptr) {
    struct kefir_list phi_queue;
    REQUIRE_OK(kefir_list_init(&phi_queue));

    kefir_result_t res = insert_predecessor_block_impl(mem, structure, code, loop_entry, loop_exit, &phi_queue,
                                                       predecessor_block_id_ptr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &phi_queue);
        return res;
    });
    REQUIRE_OK(kefir_list_free(mem, &phi_queue));
    return KEFIR_OK;
}

static kefir_result_t all_inputs_hoisted(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct licm_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid LICM state"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));

    const kefir_bool_t loop_local =
        kefir_hashtreeset_has(&state->loop->loop_blocks, (kefir_hashtreeset_entry_t) instr->block_id);
    if (loop_local) {
        if (!kefir_hashtreeset_has(&state->traversal_queue_index, (kefir_hashtreeset_entry_t) instr_ref)) {
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->traversal_queue,
                                               kefir_list_tail(&state->traversal_queue),
                                               (void *) (kefir_uptr_t) instr_ref));
            REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->traversal_queue_index,
                                             (kefir_hashtreeset_entry_t) instr_ref));
        }
        state->all_inputs_nonlocal = false;
    }
    return KEFIR_OK;
}

static kefir_result_t do_hoist(struct licm_state *state, kefir_opt_block_id_t loop_entry,
                               kefir_opt_block_id_t loop_exit, kefir_opt_block_id_t *hoist_target) {
    for (struct kefir_list_entry *iter = kefir_list_head(&state->traversal_queue); iter != NULL;
         iter = kefir_list_head(&state->traversal_queue)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(state->mem, &state->traversal_queue, iter));
        REQUIRE_OK(
            kefir_hashtreeset_delete(state->mem, &state->traversal_queue_index, (kefir_hashtreeset_entry_t) instr_ref));

        const struct kefir_opt_instruction *instr;
        kefir_result_t res = kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        state->all_inputs_nonlocal = true;
        REQUIRE_OK(kefir_opt_instruction_extract_inputs(&state->func->code, instr, true, all_inputs_hoisted, state));
        if (!state->all_inputs_nonlocal) {
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->traversal_queue,
                                               kefir_list_tail(&state->traversal_queue),
                                               (void *) (kefir_uptr_t) instr_ref));
            REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->traversal_queue_index,
                                             (kefir_hashtreeset_entry_t) instr_ref));
            continue;
        }

        if (*hoist_target == KEFIR_ID_NONE) {
            REQUIRE_OK(insert_predecessor_block(state->mem, &state->structure, &state->func->code, loop_entry,
                                                loop_exit, hoist_target));
            res = kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr);
            if (res == KEFIR_NOT_FOUND) {
                continue;
            }
            REQUIRE_OK(res);
        }
        kefir_bool_t can_hoist;
        REQUIRE_OK(kefir_opt_can_hoist_instruction(&state->structure, instr_ref, *hoist_target, &can_hoist));
        if (can_hoist) {
            REQUIRE_OK(kefir_opt_move_instruction(state->mem, &state->func->code, &state->func->debug_info, instr_ref,
                                                  *hoist_target, &instr_ref));
        } else {
            break;
        }
    }
    return KEFIR_OK;
}

static kefir_result_t process_loop(struct licm_state *state) {
    REQUIRE_OK(kefir_hashtreeset_clean(state->mem, &state->processed_instr));

    kefir_result_t res;
    struct kefir_hashtreeset_iterator iter;
    REQUIRE_OK(kefir_list_clear(state->mem, &state->traversal_queue));
    REQUIRE_OK(kefir_hashtreeset_clean(state->mem, &state->traversal_queue_index));
    for (res = kefir_hashtreeset_iter(&state->loop->loop_blocks, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) iter.entry);
        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&state->func->code, block_id, &block));

        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_control_head(&state->func->code, block, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_control(&state->func->code, instr_ref, &instr_ref)) {
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->traversal_queue,
                                               kefir_list_tail(&state->traversal_queue),
                                               (void *) (kefir_uptr_t) instr_ref));
            REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->traversal_queue_index,
                                             (kefir_hashtreeset_entry_t) instr_ref));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (struct kefir_list_entry *iter = kefir_list_head(&state->traversal_queue); iter != NULL;
         iter = kefir_list_head(&state->traversal_queue)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, instr_ref, (kefir_uptr_t) iter->value);
        REQUIRE_OK(kefir_list_pop(state->mem, &state->traversal_queue, iter));
        REQUIRE_OK(
            kefir_hashtreeset_delete(state->mem, &state->traversal_queue_index, (kefir_hashtreeset_entry_t) instr_ref));
        if (kefir_hashtreeset_has(&state->processed_instr, (kefir_hashtreeset_entry_t) instr_ref)) {
            continue;
        }

        const struct kefir_opt_instruction *instr;
        res = kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        state->all_inputs_processed = true;
        state->all_inputs_nonlocal = true;
        REQUIRE_OK(kefir_opt_instruction_extract_inputs(&state->func->code, instr, false, all_inputs_processed, state));

        if (!state->all_inputs_processed && instr->operation.opcode != KEFIR_OPT_OPCODE_PHI) {
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->traversal_queue,
                                               kefir_list_tail(&state->traversal_queue),
                                               (void *) (kefir_uptr_t) instr_ref));
            REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->traversal_queue_index,
                                             (kefir_hashtreeset_entry_t) instr_ref));
            continue;
        }

        REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->processed_instr, (kefir_hashtreeset_entry_t) instr_ref));
        if (!state->all_inputs_nonlocal) {
            continue;
        }

        kefir_bool_t is_side_effect_free, is_control_flow;
        REQUIRE_OK(kefir_opt_instruction_is_side_effect_free(instr, &is_side_effect_free));
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(state->structure.code, instr_ref, &is_control_flow));
        if (is_side_effect_free && !is_control_flow) {
            REQUIRE_OK(
                kefir_hashtreeset_add(state->mem, &state->hoist_candidates, (kefir_hashtreeset_entry_t) instr_ref));
        }
    }

    kefir_opt_block_id_t hoist_target = KEFIR_ID_NONE;
    for (res = kefir_hashtreeset_iter(&state->hoist_candidates, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, candidate_instr_ref, (kefir_uptr_t) iter.entry);

        const struct kefir_opt_instruction *candidate_instr;
        res = kefir_opt_code_container_instr(&state->func->code, candidate_instr_ref, &candidate_instr);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        switch (candidate_instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_INT_CONST:
            case KEFIR_OPT_OPCODE_UINT_CONST:
            case KEFIR_OPT_OPCODE_FLOAT32_CONST:
            case KEFIR_OPT_OPCODE_FLOAT64_CONST:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_CONST:
            case KEFIR_OPT_OPCODE_STRING_REF:
            case KEFIR_OPT_OPCODE_BLOCK_LABEL:
            case KEFIR_OPT_OPCODE_INT_PLACEHOLDER:
            case KEFIR_OPT_OPCODE_FLOAT32_PLACEHOLDER:
            case KEFIR_OPT_OPCODE_FLOAT64_PLACEHOLDER:
            case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_8BITS:
            case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_16BITS:
            case KEFIR_OPT_OPCODE_INT64_SIGN_EXTEND_32BITS:
            case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_8BITS:
            case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_16BITS:
            case KEFIR_OPT_OPCODE_INT64_ZERO_EXTEND_32BITS:
                // Intentionally left blank
                break;

            default:
                REQUIRE_OK(kefir_list_clear(state->mem, &state->traversal_queue));
                REQUIRE_OK(kefir_hashtreeset_clean(state->mem, &state->traversal_queue_index));
                REQUIRE_OK(kefir_list_insert_after(state->mem, &state->traversal_queue,
                                                   kefir_list_tail(&state->traversal_queue),
                                                   (void *) (kefir_uptr_t) candidate_instr_ref));
                REQUIRE_OK(kefir_hashtreeset_add(state->mem, &state->traversal_queue_index,
                                                 (kefir_hashtreeset_entry_t) candidate_instr_ref));
                REQUIRE_OK(
                    do_hoist(state, state->loop->loop_entry_block_id, state->loop->loop_exit_block_id, &hoist_target));
                break;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t process_nest(struct licm_state *state, const struct kefir_tree_node *nest) {
    state->loop = nest->value;
    REQUIRE_OK(process_loop(state));

    for (struct kefir_tree_node *child = kefir_tree_first_child(nest); child != NULL;
         child = kefir_tree_next_sibling(child)) {
        REQUIRE_OK(process_nest(state, child));
    }
    return KEFIR_OK;
}

static kefir_result_t licm_impl(struct licm_state *state) {
    REQUIRE_OK(kefir_opt_code_structure_build(state->mem, &state->structure, &state->func->code));
    REQUIRE_OK(kefir_opt_code_loop_collection_build(state->mem, &state->loops, &state->structure));

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
    REQUIRE_OK(kefir_hashtreeset_init(&state.processed_instr, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.hoist_candidates, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_hashtreeset_init(&state.traversal_queue_index, &kefir_hashtree_uint_ops));
    REQUIRE_OK(kefir_list_init(&state.traversal_queue));
    REQUIRE_OK(kefir_opt_code_structure_init(&state.structure));
    REQUIRE_OK(kefir_opt_code_loop_collection_init(&state.loops));

    kefir_result_t res = licm_impl(&state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_loop_collection_free(mem, &state.loops);
        kefir_opt_code_structure_free(mem, &state.structure);
        kefir_list_free(mem, &state.traversal_queue);
        kefir_hashtreeset_free(mem, &state.hoist_candidates);
        kefir_hashtreeset_free(mem, &state.traversal_queue_index);
        kefir_hashtreeset_free(mem, &state.processed_instr);
        return res;
    });
    res = kefir_opt_code_loop_collection_free(mem, &state.loops);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_structure_free(mem, &state.structure);
        kefir_list_free(mem, &state.traversal_queue);
        kefir_hashtreeset_free(mem, &state.hoist_candidates);
        kefir_hashtreeset_free(mem, &state.traversal_queue_index);
        kefir_hashtreeset_free(mem, &state.processed_instr);
        return res;
    });
    res = kefir_opt_code_structure_free(mem, &state.structure);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.traversal_queue);
        kefir_hashtreeset_free(mem, &state.traversal_queue_index);
        kefir_hashtreeset_free(mem, &state.hoist_candidates);
        kefir_hashtreeset_free(mem, &state.processed_instr);
        return res;
    });
    res = kefir_list_free(mem, &state.traversal_queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &state.traversal_queue_index);
        kefir_hashtreeset_free(mem, &state.hoist_candidates);
        kefir_hashtreeset_free(mem, &state.processed_instr);
        return res;
    });
    res = kefir_hashtreeset_free(mem, &state.traversal_queue_index);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &state.hoist_candidates);
        kefir_hashtreeset_free(mem, &state.processed_instr);
        return res;
    });
    res = kefir_hashtreeset_free(mem, &state.hoist_candidates);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtreeset_free(mem, &state.processed_instr);
        return res;
    });
    REQUIRE_OK(kefir_hashtreeset_free(mem, &state.processed_instr));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassLoopInvariantCodeMotion = {
    .name = "licm", .apply = loop_invariant_code_motion_apply, .payload = NULL};

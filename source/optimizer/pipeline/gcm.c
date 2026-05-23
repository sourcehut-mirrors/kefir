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
#include "kefir/optimizer/control_flow.h"
#include "kefir/optimizer/loop_nest.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct gcm_state {
    struct kefir_mem *mem;
    struct kefir_opt_code_container *code;
    struct kefir_opt_code_debug_info *debug_info;
    struct kefir_opt_code_control_flow control_flow;
    struct kefir_opt_code_loop_collection loops;

    struct kefir_list queue;
    struct kefir_hashset scheduled_early;
    struct kefir_hashset scheduled_late;

    kefir_opt_block_id_t deepest_input_block_ref;
};

static kefir_result_t gcm_schedule_early_inputs_phase1(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct gcm_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid GCM state"));

    if (!kefir_hashset_has(&state->scheduled_early, (kefir_hashset_key_t) instr_ref)) {
        kefir_uint64_t key = (kefir_uint32_t) instr_ref;
        REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, NULL, (void *) (kefir_uptr_t) key));
    }
    return KEFIR_OK;
}

static kefir_result_t gcm_schedule_early_inputs_phase2(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct gcm_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid GCM state"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(state->code, instr_ref, &instr));

    if (state->control_flow.blocks[state->deepest_input_block_ref].dominance_tree_level <
        state->control_flow.blocks[instr->block_id].dominance_tree_level) {
        state->deepest_input_block_ref = instr->block_id;
    }
    return KEFIR_OK;
}

static kefir_result_t gcm_schedule_early(struct gcm_state *state) {
    for (kefir_opt_block_id_t block_ref = 0; block_ref < kefir_opt_code_container_block_count(state->code);
         block_ref++) {
        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_head(state->code, block_ref, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_sibling(state->code, instr_ref, &instr_ref)) {
            kefir_uint64_t key = (kefir_uint32_t) instr_ref;
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, kefir_list_tail(&state->queue),
                                               (void *) (kefir_uptr_t) key));
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }

    for (struct kefir_list_entry *iter = kefir_list_head(&state->queue); iter != NULL;
         iter = kefir_list_head(&state->queue)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, key, (kefir_uptr_t) iter->value);
        kefir_opt_instruction_ref_t instr_ref = (kefir_uint32_t) key;
        kefir_bool_t inputs_resolved = (key >> 32) != 0;

        REQUIRE_OK(kefir_list_pop(state->mem, &state->queue, iter));
        if (kefir_hashset_has(&state->scheduled_early, (kefir_hashset_key_t) instr_ref)) {
            continue;
        }

        const struct kefir_opt_instruction *instr;
        kefir_result_t res = kefir_opt_code_container_instr(state->code, instr_ref, &instr);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        if (!inputs_resolved) {
            kefir_uint64_t key = (1ull << 32) | (kefir_uint32_t) instr_ref;
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, NULL, (void *) (kefir_uptr_t) key));

            REQUIRE_OK(kefir_opt_instruction_extract_inputs(state->code, instr, true, gcm_schedule_early_inputs_phase1,
                                                            state));
            continue;
        }

        kefir_bool_t moveable;
        REQUIRE_OK(kefir_opt_instruction_is_moveable(state->code, instr_ref, &moveable));
        if (moveable) {
            state->deepest_input_block_ref = state->code->entry_point;
            REQUIRE_OK(kefir_opt_instruction_extract_inputs(state->code, instr, true, gcm_schedule_early_inputs_phase2,
                                                            state));
            if (state->deepest_input_block_ref != state->code->gate_block) {
                REQUIRE_OK(res);
                REQUIRE_OK(kefir_opt_move_instruction(state->code, instr_ref, state->deepest_input_block_ref));
            }
        }
        REQUIRE_OK(kefir_hashset_add(state->mem, &state->scheduled_early, (kefir_hashtable_key_t) instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t do_schedule_instr(struct gcm_state *state, kefir_opt_instruction_ref_t instr_ref,
                                        kefir_opt_block_id_t late_block_ref) {
    REQUIRE(late_block_ref != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(state->code, instr_ref, &instr));

    kefir_opt_block_id_t best_block_ref = late_block_ref;

    kefir_uint32_t iter_loop_level, best_loop_level;
    REQUIRE_OK(kefir_opt_code_loop_level(&state->loops, best_block_ref, &best_loop_level));
    for (kefir_opt_block_id_t iter_ref = late_block_ref;;
         iter_ref = state->control_flow.blocks[iter_ref].immediate_dominator) {
        if (iter_ref == KEFIR_ID_NONE) {
            best_block_ref = instr->block_id;
            break;
        }

        REQUIRE_OK(kefir_opt_code_loop_level(&state->loops, iter_ref, &iter_loop_level));

        if (iter_loop_level < best_loop_level && iter_ref != state->code->gate_block) {
            best_block_ref = iter_ref;
            best_loop_level = iter_loop_level;
        }
        if (iter_ref == instr->block_id) {
            break;
        }
    }

    if (best_block_ref != instr->block_id && best_block_ref != state->code->gate_block) {
        REQUIRE_OK(kefir_opt_move_instruction(state->code, instr_ref, best_block_ref));
    }

    return KEFIR_OK;
}

static kefir_result_t gcm_schedule_late(struct gcm_state *state) {
    REQUIRE_OK(kefir_list_clear(state->mem, &state->queue));

    kefir_result_t res;
    struct kefir_hashset_iterator early_sched_iter;
    kefir_hashset_key_t iter_key;
    for (res = kefir_hashset_iter(&state->scheduled_early, &early_sched_iter, &iter_key); res == KEFIR_OK;
         res = kefir_hashset_next(&early_sched_iter, &iter_key)) {
        REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, NULL,
                                           (void *) (kefir_uptr_t) (kefir_uint32_t) iter_key));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    for (struct kefir_list_entry *iter = kefir_list_head(&state->queue); iter != NULL;
         iter = kefir_list_head(&state->queue)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, key, (kefir_uptr_t) iter->value);
        kefir_opt_instruction_ref_t instr_ref = (kefir_uint32_t) key;
        kefir_bool_t uses_resolved = (key >> 32) != 0;

        REQUIRE_OK(kefir_list_pop(state->mem, &state->queue, iter));
        if (kefir_hashset_has(&state->scheduled_late, (kefir_hashset_key_t) instr_ref)) {
            continue;
        }

        const struct kefir_opt_instruction *instr;
        kefir_result_t res = kefir_opt_code_container_instr(state->code, instr_ref, &instr);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);

        if (!uses_resolved && instr->operation.opcode != KEFIR_OPT_OPCODE_PHI) {
            kefir_uint64_t key = (1ull << 32) | (kefir_uint32_t) instr_ref;
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, NULL, (void *) (kefir_uptr_t) key));

            kefir_result_t res;
            struct kefir_opt_instruction_use_iterator use_iter;
            for (res = kefir_opt_code_container_instruction_use_instr_iter(state->code, instr_ref, &use_iter);
                 res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
                if (!kefir_hashset_has(&state->scheduled_late, (kefir_hashset_key_t) use_iter.use_instr_ref)) {
                    REQUIRE_OK(
                        kefir_list_insert_after(state->mem, &state->queue, NULL,
                                                (void *) (kefir_uptr_t) (kefir_uint32_t) use_iter.use_instr_ref));
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
            continue;
        }

        REQUIRE_OK(kefir_hashset_add(state->mem, &state->scheduled_late, (kefir_hashset_key_t) instr_ref));

        kefir_bool_t moveable;
        REQUIRE_OK(kefir_opt_instruction_is_moveable(state->code, instr_ref, &moveable));
        if (!moveable) {
            continue;
        }

        kefir_opt_block_id_t late_schedule_block_ref = KEFIR_ID_NONE;

        struct kefir_opt_instruction_use_iterator use_iter;
        for (res = kefir_opt_code_container_instruction_use_instr_iter(state->code, instr_ref, &use_iter);
             res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {

            const struct kefir_opt_instruction *use_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(state->code, use_iter.use_instr_ref, &use_instr));
            kefir_bool_t reachable;
            REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(&state->control_flow, use_instr->block_id,
                                                                           &reachable));
            if (!reachable) {
                continue;
            }
            if (use_instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
                struct kefir_opt_phi_node_link_iterator link_iter;
                kefir_opt_block_id_t link_block_id;
                kefir_opt_instruction_ref_t link_instr_ref;
                for (res = kefir_opt_phi_node_link_iter(state->code, use_instr->id, &link_iter, &link_block_id,
                                                        &link_instr_ref);
                     res == KEFIR_OK; res = kefir_opt_phi_node_link_next(&link_iter, &link_block_id, &link_instr_ref)) {
                    REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(&state->control_flow, link_block_id,
                                                                                   &reachable));
                    if (!reachable) {
                        continue;
                    }
                    if (link_instr_ref == instr_ref) {
                        if (late_schedule_block_ref == KEFIR_ID_NONE) {
                            late_schedule_block_ref = link_block_id;
                        } else {
                            REQUIRE_OK(kefir_opt_find_closest_common_dominator(&state->control_flow,
                                                                               late_schedule_block_ref, link_block_id,
                                                                               &late_schedule_block_ref));
                        }
                    }
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
            } else {
                if (late_schedule_block_ref == KEFIR_ID_NONE) {
                    late_schedule_block_ref = use_instr->block_id;
                } else {
                    REQUIRE_OK(kefir_opt_find_closest_common_dominator(&state->control_flow, late_schedule_block_ref,
                                                                       use_instr->block_id, &late_schedule_block_ref));
                }
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
        REQUIRE_OK(do_schedule_instr(state, instr_ref, late_schedule_block_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t gcm_impl(struct gcm_state *state) {
    REQUIRE_OK(kefir_opt_code_control_flow_build(state->mem, &state->control_flow, state->code));
    REQUIRE_OK(kefir_opt_code_loop_collection_build(state->mem, &state->loops, &state->control_flow));
    REQUIRE_OK(gcm_schedule_early(state));
    REQUIRE_OK(gcm_schedule_late(state));
    return KEFIR_OK;
}

static kefir_result_t global_code_motion_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                               struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                               const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct gcm_state state = {.mem = mem, .code = &func->code, .debug_info = &func->debug_info};
    REQUIRE_OK(kefir_opt_code_control_flow_init(&state.control_flow));
    REQUIRE_OK(kefir_opt_code_loop_collection_init(&state.loops));
    REQUIRE_OK(kefir_list_init(&state.queue));
    REQUIRE_OK(kefir_hashset_init(&state.scheduled_early, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&state.scheduled_late, &kefir_hashtable_uint_ops));

    kefir_result_t res = gcm_impl(&state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.scheduled_late);
        kefir_hashset_free(mem, &state.scheduled_early);
        kefir_list_free(mem, &state.queue);
        kefir_opt_code_loop_collection_free(mem, &state.loops);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_hashset_free(mem, &state.scheduled_late);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &state.scheduled_early);
        kefir_list_free(mem, &state.queue);
        kefir_opt_code_loop_collection_free(mem, &state.loops);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_hashset_free(mem, &state.scheduled_early);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_list_free(mem, &state.queue);
        kefir_opt_code_loop_collection_free(mem, &state.loops);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_list_free(mem, &state.queue);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_loop_collection_free(mem, &state.loops);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_opt_code_loop_collection_free(mem, &state.loops);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_control_flow_free(mem, &state.control_flow));

    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassGlobalCodeMotion = {
    .name = "gcm", .apply = global_code_motion_apply, .payload = NULL};

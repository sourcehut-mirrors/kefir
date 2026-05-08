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
    struct kefir_opt_code_control_flow control_flow;
    struct kefir_opt_code_loop_collection loops;

    struct kefir_list queue;
    struct kefir_hashtable early_schedule;
    struct kefir_hashtable late_schedule;

    kefir_opt_block_id_t deepest_input_block_ref;
};

static kefir_result_t gcm_schedule_early_inputs_phase1(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct gcm_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid GCM state"));

    if (!kefir_hashtable_has(&state->early_schedule, (kefir_hashtable_key_t) instr_ref)) {
        kefir_uint64_t key = (kefir_uint32_t) instr_ref;
        REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, NULL, (void *) (kefir_uptr_t) key));
    }
    return KEFIR_OK;
}

static kefir_result_t gcm_schedule_early_inputs_phase2(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct gcm_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid GCM state"));

    kefir_hashtable_value_t table_value;
    REQUIRE_OK(kefir_hashtable_at(&state->early_schedule, (kefir_hashtable_key_t) instr_ref, &table_value));

    if (state->control_flow.blocks[state->deepest_input_block_ref].dominance_tree_level <
        state->control_flow.blocks[table_value].dominance_tree_level) {
        state->deepest_input_block_ref = table_value;
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
        if (kefir_hashtable_has(&state->early_schedule, (kefir_hashtable_key_t) instr_ref)) {
            continue;
        }

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(state->code, instr_ref, &instr));

        if (!inputs_resolved) {
            kefir_uint64_t key = (1ull << 32) | (kefir_uint32_t) instr_ref;
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, NULL, (void *) (kefir_uptr_t) key));

            REQUIRE_OK(kefir_opt_instruction_extract_inputs(state->code, instr, true, gcm_schedule_early_inputs_phase1,
                                                            state));
            continue;
        }

        kefir_bool_t is_control_flow;
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(state->code, instr_ref, &is_control_flow));
        if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI || is_control_flow) {
            REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->early_schedule, (kefir_hashtable_key_t) instr_ref,
                                              (kefir_hashtable_value_t) instr->block_id));
        } else {
            state->deepest_input_block_ref = state->code->entry_point;
            REQUIRE_OK(kefir_opt_instruction_extract_inputs(state->code, instr, true, gcm_schedule_early_inputs_phase2,
                                                            state));
            REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->early_schedule, (kefir_hashtable_key_t) instr_ref,
                                              (kefir_hashtable_value_t) state->deepest_input_block_ref));
        }
    }
    return KEFIR_OK;
}

static kefir_result_t gcm_schedule_late(struct gcm_state *state) {
    REQUIRE_OK(kefir_list_clear(state->mem, &state->queue));

    kefir_result_t res;
    struct kefir_hashtable_iterator early_sched_iter;
    kefir_hashtable_key_t iter_key;
    for (res = kefir_hashtable_iter(&state->early_schedule, &early_sched_iter, &iter_key, NULL); res == KEFIR_OK;
         res = kefir_hashtable_next(&early_sched_iter, &iter_key, NULL)) {
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
        if (kefir_hashtable_has(&state->late_schedule, (kefir_hashtable_key_t) instr_ref)) {
            continue;
        }

        const struct kefir_opt_instruction *instr;
        REQUIRE_OK(kefir_opt_code_container_instr(state->code, instr_ref, &instr));

        if (!uses_resolved && instr->operation.opcode != KEFIR_OPT_OPCODE_PHI) {
            kefir_uint64_t key = (1ull << 32) | (kefir_uint32_t) instr_ref;
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, NULL, (void *) (kefir_uptr_t) key));

            kefir_result_t res;
            struct kefir_opt_instruction_use_iterator use_iter;
            for (res = kefir_opt_code_container_instruction_use_instr_iter(state->code, instr_ref, &use_iter);
                 res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
                if (!kefir_hashtable_has(&state->late_schedule, (kefir_hashtable_key_t) use_iter.use_instr_ref)) {
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

        kefir_bool_t is_control_flow;
        REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(state->code, instr_ref, &is_control_flow));
        if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI || is_control_flow) {
            REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->late_schedule, (kefir_hashtable_key_t) instr_ref,
                                              (kefir_hashtable_value_t) instr->block_id));
            continue;
        }

        kefir_opt_block_id_t late_schedule_block_ref = KEFIR_ID_NONE;

        kefir_result_t res;
        struct kefir_opt_instruction_use_iterator use_iter;
        for (res = kefir_opt_code_container_instruction_use_instr_iter(state->code, instr_ref, &use_iter);
             res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {

            const struct kefir_opt_instruction *use_instr;
            REQUIRE_OK(kefir_opt_code_container_instr(state->code, use_iter.use_instr_ref, &use_instr));
            if (use_instr->operation.opcode == KEFIR_OPT_OPCODE_PHI) {
                struct kefir_opt_phi_node_link_iterator link_iter;
                kefir_opt_block_id_t link_block_id;
                for (res = kefir_opt_phi_node_link_iter(state->code, use_instr->id, &link_iter, &link_block_id, NULL);
                     res == KEFIR_OK; res = kefir_opt_phi_node_link_next(&link_iter, &link_block_id, NULL)) {
                    if (late_schedule_block_ref == KEFIR_ID_NONE) {
                        late_schedule_block_ref = link_block_id;
                    } else {
                        REQUIRE_OK(kefir_opt_find_closest_common_dominator(
                            &state->control_flow, late_schedule_block_ref, link_block_id, &late_schedule_block_ref));
                    }
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }
            } else {
                kefir_hashtable_value_t table_value;
                REQUIRE_OK(kefir_hashtable_at(&state->late_schedule, (kefir_hashtable_key_t) use_iter.use_instr_ref,
                                              &table_value));
                ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, table_value);
                if (late_schedule_block_ref == KEFIR_ID_NONE) {
                    late_schedule_block_ref = block_id;
                } else {
                    REQUIRE_OK(kefir_opt_find_closest_common_dominator(&state->control_flow, late_schedule_block_ref,
                                                                       block_id, &late_schedule_block_ref));
                }
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        REQUIRE_OK(kefir_hashtable_insert(state->mem, &state->late_schedule, (kefir_hashtable_key_t) instr_ref,
                                          (kefir_hashtable_value_t) late_schedule_block_ref));
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

    struct gcm_state state = {.mem = mem, .code = &func->code};
    REQUIRE_OK(kefir_opt_code_control_flow_init(&state.control_flow));
    REQUIRE_OK(kefir_opt_code_loop_collection_init(&state.loops));
    REQUIRE_OK(kefir_list_init(&state.queue));
    REQUIRE_OK(kefir_hashtable_init(&state.early_schedule, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashtable_init(&state.late_schedule, &kefir_hashtable_uint_ops));

    kefir_result_t res = gcm_impl(&state);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.late_schedule);
        kefir_hashtable_free(mem, &state.early_schedule);
        kefir_list_free(mem, &state.queue);
        kefir_opt_code_loop_collection_free(mem, &state.loops);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.late_schedule);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashtable_free(mem, &state.early_schedule);
        kefir_list_free(mem, &state.queue);
        kefir_opt_code_loop_collection_free(mem, &state.loops);
        kefir_opt_code_control_flow_free(mem, &state.control_flow);
        return res;
    });
    res = kefir_hashtable_free(mem, &state.early_schedule);
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

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
#include "kefir/optimizer/memory_ssa.h"
#include "kefir/optimizer/iteration_space.h"
#include "kefir/optimizer/mem2reg_util.h"
#include "kefir/optimizer/escape.h"
#include "kefir/optimizer/alias.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct licm_state {
    struct kefir_mem *mem;
    struct kefir_opt_module *module;
    struct kefir_opt_function *func;
    struct kefir_opt_code_control_flow control_flow;
    struct kefir_opt_code_liveness liveness;
    struct kefir_opt_code_escape_analysis escapes;
    struct kefir_opt_code_loop_collection loops;

    const struct kefir_opt_code_loop *loop;
    struct kefir_hashset loop_memory_ops;

    struct kefir_list queue;
    struct kefir_hashset processed;
    kefir_opt_instruction_ref_t deepest_input_block_ref;
};

static kefir_result_t collect_loop_memory_accesses(struct licm_state *state) {
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->loop_memory_ops));

    for (kefir_opt_block_id_t block_ref = 0; block_ref < kefir_opt_code_container_block_count(&state->func->code);
         block_ref++) {
        kefir_result_t res;
        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_head(&state->func->code, block_ref, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_sibling(&state->func->code, instr_ref, &instr_ref)) {
            const struct kefir_opt_instruction *instr;
            REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));

            kefir_uint32_t op_props;
            REQUIRE_OK(kefir_opt_memssa_util_is_instr_memory(instr, &op_props));

            if (op_props != KEFIR_OPT_MEMSSA_MEMORY_OP_NONE) {
                REQUIRE_OK(kefir_hashset_add(state->mem, &state->loop_memory_ops, (kefir_hashset_key_t) instr_ref));
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }
    }
    return KEFIR_OK;
}

static kefir_result_t hoist_memory_operation(struct licm_state *state, kefir_opt_instruction_ref_t instr_ref,
                                             kefir_opt_block_id_t hoist_target) {
    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));
    REQUIRE(instr->block_id != hoist_target, KEFIR_OK);

    kefir_opt_instruction_ref_t location1_ref = KEFIR_ID_NONE, location2_ref = KEFIR_ID_NONE;
    kefir_size_t size1 = 0, size2 = 0;
    kefir_int64_t offset1 = 0, offset2 = 0;

    kefir_result_t res = kefir_opt_code_util_classify_memory_access(instr, &location1_ref, &size1, &offset1);
    if (res == KEFIR_NO_MATCH) {
        location1_ref = instr_ref;
        size1 = 0;
        offset1 = 0;
        res = KEFIR_OK;
    }
    REQUIRE_OK(res);

    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t key;
    for (res = kefir_hashset_iter(&state->loop_memory_ops, &iter, &key); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &key)) {
        ASSIGN_DECL_CAST(kefir_opt_instruction_ref_t, other_instr_ref, key);
        if (instr_ref == other_instr_ref) {
            continue;
        }
        const struct kefir_opt_instruction *other_instr;
        REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, other_instr_ref, &other_instr));

        res = kefir_opt_code_util_classify_memory_access(other_instr, &location2_ref, &size2, &offset2);
        if (res == KEFIR_NO_MATCH) {
            location2_ref = other_instr_ref;
            size2 = 0;
            offset2 = 0;
            res = KEFIR_OK;
        }
        REQUIRE_OK(res);

        kefir_bool_t may_alias;
        REQUIRE_OK(kefir_opt_code_may_alias(&state->func->code, &state->escapes, state->module->ir_module,
                                            location1_ref, size1, offset1, location2_ref, size2, offset2, &may_alias));
        if (may_alias) {
            return KEFIR_OK;
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    kefir_bool_t is_control_flow;
    REQUIRE_OK(kefir_opt_code_instruction_is_control_flow(&state->func->code, instr_ref, &is_control_flow));
    REQUIRE(is_control_flow, KEFIR_OK);

    REQUIRE_OK(kefir_opt_code_container_drop_control(&state->func->code, instr_ref));
    REQUIRE_OK(kefir_opt_move_instruction(&state->func->code, instr_ref, hoist_target));

    kefir_opt_instruction_ref_t insert_control_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&state->func->code, hoist_target, &insert_control_ref));
    if (insert_control_ref != KEFIR_ID_NONE) {
        REQUIRE_OK(kefir_opt_instruction_prev_control(&state->func->code, insert_control_ref, &insert_control_ref));
    }
    REQUIRE_OK(
        kefir_opt_code_container_insert_control(&state->func->code, hoist_target, insert_control_ref, instr_ref));
    return KEFIR_OK;
}

static kefir_result_t is_safe_load(struct licm_state *state, kefir_opt_instruction_ref_t location_ref,
                                   kefir_bool_t pessimistic, kefir_bool_t *safe_load) {
    const struct kefir_opt_instruction *location;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, location_ref, &location));

    *safe_load = false;
    if (location->operation.opcode == KEFIR_OPT_OPCODE_GET_GLOBAL ||
        location->operation.opcode == KEFIR_OPT_OPCODE_GET_THREAD_LOCAL ||
        location->operation.opcode == KEFIR_OPT_OPCODE_ALLOC_LOCAL ||
        location->operation.opcode == KEFIR_OPT_OPCODE_REF_LOCAL) {
        *safe_load = true;
    } else if (location->operation.opcode == KEFIR_OPT_OPCODE_INT64_ADD && !pessimistic) {
        kefir_bool_t safe_left = false, safe_right = false;
        REQUIRE_OK(is_safe_load(state, location->operation.parameters.refs[0], true, &safe_left));
        REQUIRE_OK(is_safe_load(state, location->operation.parameters.refs[1], true, &safe_right));
        *safe_load = safe_left || safe_right;
    } else if (location->operation.opcode == KEFIR_OPT_OPCODE_INT64_SUB && !pessimistic) {
        REQUIRE_OK(is_safe_load(state, location->operation.parameters.refs[0], true, safe_load));
    }
    return KEFIR_OK;
}

static kefir_result_t licm_scan_inputs_phase1(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct licm_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid GCM state"));

    if (!kefir_hashset_has(&state->processed, (kefir_hashset_key_t) instr_ref)) {
        kefir_uint64_t key = (kefir_uint32_t) instr_ref;
        REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, NULL, (void *) (kefir_uptr_t) key));
    }
    return KEFIR_OK;
}

static kefir_result_t licm_scan_inputs_phase2(kefir_opt_instruction_ref_t instr_ref, void *payload) {
    ASSIGN_DECL_CAST(struct licm_state *, state, payload);
    REQUIRE(state != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid GCM state"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));

    if (state->control_flow.blocks[state->deepest_input_block_ref].dominance_tree_level <
        state->control_flow.blocks[instr->block_id].dominance_tree_level) {
        state->deepest_input_block_ref = instr->block_id;
    }
    return KEFIR_OK;
}

static kefir_result_t process_loop_initialize_queue(struct licm_state *state) {
    kefir_result_t res;
    struct kefir_hashset_iterator iter;
    kefir_hashset_key_t key;
    for (res = kefir_hashset_iter(&state->loop->blocks, &iter, &key); res == KEFIR_OK;
         res = kefir_hashset_next(&iter, &key)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, (kefir_uptr_t) key);
        kefir_bool_t is_reachable;
        REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(&state->control_flow, block_id, &is_reachable));
        if (!is_reachable) {
            continue;
        }

        const struct kefir_opt_code_block *block;
        REQUIRE_OK(kefir_opt_code_container_block(&state->func->code, block_id, &block));

        kefir_opt_instruction_ref_t instr_ref;
        for (res = kefir_opt_code_block_instr_control_head(&state->func->code, block_id, &instr_ref);
             res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
             res = kefir_opt_instruction_next_control(&state->func->code, instr_ref, &instr_ref)) {
            kefir_uint64_t key = (kefir_uint32_t) instr_ref;
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, kefir_list_tail(&state->queue),
                                               (void *) (kefir_uptr_t) key));
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }

    return KEFIR_OK;
}

static kefir_result_t process_loop_impl(struct licm_state *state) {
    kefir_result_t res;
    kefir_bool_t must_execute = false;
    struct kefir_opt_loop_iteration_space iteration_space;
    res = kefir_opt_loop_match_iteration_space(&state->func->code, state->loop, &iteration_space);
    if (res != KEFIR_NO_MATCH) {
        REQUIRE_OK(res);
        REQUIRE_OK(kefir_opt_loop_must_execute(&state->func->code, &iteration_space, &must_execute));
    }

    for (struct kefir_list_entry *iter = kefir_list_head(&state->queue); iter != NULL;
         iter = kefir_list_head(&state->queue)) {
        ASSIGN_DECL_CAST(kefir_uint64_t, key, (kefir_uptr_t) iter->value);
        kefir_opt_instruction_ref_t instr_ref = (kefir_uint32_t) key;
        kefir_bool_t inputs_resolved = (key >> 32) != 0;

        REQUIRE_OK(kefir_list_pop(state->mem, &state->queue, iter));
        if (kefir_hashset_has(&state->processed, (kefir_hashset_key_t) instr_ref)) {
            continue;
        }

        const struct kefir_opt_instruction *instr;
        res = kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr);
        if (res == KEFIR_NOT_FOUND) {
            continue;
        }
        REQUIRE_OK(res);
        if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI ||
            instr->operation.opcode == KEFIR_OPT_OPCODE_GET_ARGUMENT ||
            !kefir_hashset_has(&state->loop->blocks, (kefir_hashset_key_t) instr->block_id)) {
            continue;
        }

        if (!inputs_resolved) {
            kefir_uint64_t key = (1ull << 32) | (kefir_uint32_t) instr_ref;
            REQUIRE_OK(kefir_list_insert_after(state->mem, &state->queue, NULL, (void *) (kefir_uptr_t) key));

            REQUIRE_OK(
                kefir_opt_instruction_extract_inputs(&state->func->code, instr, true, licm_scan_inputs_phase1, state));
            continue;
        }

        state->deepest_input_block_ref = state->func->code.entry_point;
        REQUIRE_OK(
            kefir_opt_instruction_extract_inputs(&state->func->code, instr, true, licm_scan_inputs_phase2, state));

        switch (instr->operation.opcode) {
            case KEFIR_OPT_OPCODE_INT8_LOAD:
            case KEFIR_OPT_OPCODE_INT16_LOAD:
            case KEFIR_OPT_OPCODE_INT32_LOAD:
            case KEFIR_OPT_OPCODE_INT64_LOAD:
            case KEFIR_OPT_OPCODE_INT128_LOAD:
            case KEFIR_OPT_OPCODE_FLOAT32_LOAD:
            case KEFIR_OPT_OPCODE_FLOAT64_LOAD:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_LOAD:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD:
            case KEFIR_OPT_OPCODE_BITINT_LOAD_PRECISE:
            case KEFIR_OPT_OPCODE_DECIMAL32_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL64_LOAD:
            case KEFIR_OPT_OPCODE_DECIMAL128_LOAD:
                if (!instr->operation.parameters.memory_access.flags.volatile_access) {
                    kefir_bool_t safe_load, is_dominator;
                    REQUIRE_OK(is_safe_load(state,
                                            instr->operation.parameters.refs[KEFIR_OPT_MEMORY_ACCESS_LOCATION_REF],
                                            false, &safe_load));
                    REQUIRE_OK(kefir_opt_code_control_flow_is_dominator(&state->control_flow,
                                                                        state->loop->preheader_ref,
                                                                        state->deepest_input_block_ref, &is_dominator));
                    if (is_dominator && (must_execute || safe_load)) {
                        REQUIRE_OK(hoist_memory_operation(state, instr_ref, state->loop->preheader_ref));
                    }
                }
                break;

            case KEFIR_OPT_OPCODE_INT8_STORE:
            case KEFIR_OPT_OPCODE_INT16_STORE:
            case KEFIR_OPT_OPCODE_INT32_STORE:
            case KEFIR_OPT_OPCODE_INT64_STORE:
            case KEFIR_OPT_OPCODE_INT128_STORE:
            case KEFIR_OPT_OPCODE_FLOAT32_STORE:
            case KEFIR_OPT_OPCODE_FLOAT64_STORE:
            case KEFIR_OPT_OPCODE_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT32_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_FLOAT64_STORE:
            case KEFIR_OPT_OPCODE_COMPLEX_LONG_DOUBLE_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE:
            case KEFIR_OPT_OPCODE_BITINT_STORE_PRECISE:
            case KEFIR_OPT_OPCODE_DECIMAL32_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL64_STORE:
            case KEFIR_OPT_OPCODE_DECIMAL128_STORE:
                if (must_execute && !instr->operation.parameters.memory_access.flags.volatile_access) {
                    kefir_bool_t is_dominator;
                    REQUIRE_OK(kefir_opt_code_control_flow_is_dominator(&state->control_flow,
                                                                        state->loop->preheader_ref,
                                                                        state->deepest_input_block_ref, &is_dominator));
                    if (is_dominator) {
                        REQUIRE_OK(hoist_memory_operation(state, instr_ref, state->loop->preheader_ref));
                    }
                }
                break;

            default: {
                kefir_bool_t moveable;
                REQUIRE_OK(kefir_opt_instruction_is_moveable(&state->func->code, instr_ref, &moveable));
                if (moveable && state->deepest_input_block_ref != state->func->code.gate_block) {
                    REQUIRE_OK(res);
                    REQUIRE_OK(
                        kefir_opt_move_instruction(&state->func->code, instr_ref, state->deepest_input_block_ref));
                }
            } break;
        }

        REQUIRE_OK(kefir_hashset_add(state->mem, &state->processed, (kefir_hashtable_key_t) instr_ref));
    }
    return KEFIR_OK;
}

static kefir_result_t process_loop(struct licm_state *state) {
    REQUIRE(state->loop->header_ref != state->control_flow.code->entry_point &&
                state->loop->header_ref != state->control_flow.code->gate_block &&
                !kefir_hashset_has(&state->control_flow.indirect_jump_target_blocks,
                                   (kefir_hashset_key_t) state->loop->header_ref),
            KEFIR_OK);

    REQUIRE_OK(kefir_list_clear(state->mem, &state->queue));
    REQUIRE_OK(kefir_hashset_clear(state->mem, &state->processed));
    REQUIRE_OK(collect_loop_memory_accesses(state));
    REQUIRE_OK(process_loop_initialize_queue(state));
    REQUIRE_OK(process_loop_impl(state));

    REQUIRE_OK(kefir_opt_code_util_distribute_loop_condition_dependencies_over_phis(state->mem, &state->func->code,
                                                                                    state->loop, 8));
    return KEFIR_OK;
}

static kefir_result_t process_nest(struct licm_state *state, struct kefir_opt_code_loop *loop) {
    for (struct kefir_opt_code_loop *nested = kefir_opt_code_loop_first_child(loop); nested != NULL;
         nested = kefir_opt_code_loop_next_sibling(nested)) {
        REQUIRE_OK(process_nest(state, nested));
    }

    state->loop = loop;
    REQUIRE_OK(process_loop(state));
    return KEFIR_OK;
}

static kefir_result_t licm_impl(struct licm_state *state) {
    REQUIRE_OK(kefir_opt_code_control_flow_build(state->mem, &state->control_flow, &state->func->code));
    REQUIRE_OK(kefir_opt_code_liveness_build(state->mem, &state->liveness, &state->control_flow));
    REQUIRE_OK(kefir_opt_code_loop_collection_build(state->mem, &state->loops, &state->control_flow));
    REQUIRE_OK(kefir_opt_code_escape_analysis_build(state->mem, &state->escapes, &state->func->code));

    REQUIRE_OK(kefir_opt_code_util_insert_loop_preheaders(state->mem, &state->func->code, &state->control_flow,
                                                          &state->loops));

    REQUIRE_OK(kefir_opt_code_control_flow_reset(state->mem, &state->control_flow));
    REQUIRE_OK(kefir_opt_code_control_flow_build(state->mem, &state->control_flow, &state->func->code));

    kefir_result_t res;
    const struct kefir_opt_loop_nest *nest;
    struct kefir_opt_code_loop_nest_collection_iterator iter;
    for (res = kefir_opt_code_loop_nest_collection_iter(&state->loops, &nest, &iter); res == KEFIR_OK && nest != NULL;
         res = kefir_opt_code_loop_nest_collection_next(&nest, &iter)) {
        REQUIRE_OK(process_nest(state, kefir_opt_loop_nest_top(nest)));
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

    struct licm_state state = {.mem = mem, .func = func, .module = module};
    REQUIRE_OK(kefir_list_init(&state.queue));
    REQUIRE_OK(kefir_hashset_init(&state.processed, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_hashset_init(&state.loop_memory_ops, &kefir_hashtable_uint_ops));
    REQUIRE_OK(kefir_opt_code_control_flow_init(&state.control_flow));
    REQUIRE_OK(kefir_opt_code_liveness_init(&state.liveness));
    REQUIRE_OK(kefir_opt_code_loop_collection_init(&state.loops));
    REQUIRE_OK(kefir_opt_code_escape_analysis_init(&state.escapes));

    kefir_result_t res = licm_impl(&state);

    kefir_opt_code_escape_analysis_free(mem, &state.escapes);
    kefir_opt_code_loop_collection_free(mem, &state.loops);
    kefir_opt_code_liveness_free(mem, &state.liveness);
    kefir_opt_code_control_flow_free(mem, &state.control_flow);
    kefir_hashset_free(mem, &state.loop_memory_ops);
    kefir_hashset_free(mem, &state.processed);
    kefir_list_free(mem, &state.queue);
    return res;
}

const struct kefir_optimizer_pass KefirOptimizerPassLoopInvariantCodeMotion = {
    .name = "licm", .apply = loop_invariant_code_motion_apply, .payload = NULL};

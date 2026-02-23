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
#include "kefir/optimizer/iteration_space.h"
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
};

static kefir_result_t process_loop(struct licm_state *state) {
    REQUIRE(state->loop->loop_entry_block_id != state->control_flow.code->entry_point &&
                state->loop->loop_entry_block_id != state->control_flow.code->gate_block &&
                !kefir_hashset_has(&state->control_flow.indirect_jump_target_blocks,
                                   (kefir_hashset_key_t) state->loop->loop_entry_block_id),
            KEFIR_OK);

    kefir_opt_block_id_t exit_block_ref = KEFIR_ID_NONE;

    kefir_opt_instruction_ref_t entry_block_tail_ref;
    REQUIRE_OK(kefir_opt_code_block_instr_control_tail(&state->func->code, state->loop->loop_entry_block_id,
                                                       &entry_block_tail_ref));
    REQUIRE(entry_block_tail_ref != KEFIR_ID_NONE, KEFIR_OK);

    const struct kefir_opt_instruction *entry_block_tail;
    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, entry_block_tail_ref, &entry_block_tail));
    REQUIRE(entry_block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH ||
                entry_block_tail->operation.opcode == KEFIR_OPT_OPCODE_BRANCH_COMPARE,
            KEFIR_OK);

    struct kefir_opt_loop_iteration_space iteration_space;
    kefir_result_t res = kefir_opt_loop_match_iteration_space(&state->func->code, state->loop, &iteration_space);
    REQUIRE(res != KEFIR_NO_MATCH, KEFIR_OK);
    REQUIRE_OK(res);

    kefir_bool_t may_execute;
    REQUIRE_OK(kefir_opt_loop_may_execute(&state->func->code, &iteration_space, &may_execute));

    struct kefir_hashtreeset_iterator iter;
    for (res = kefir_hashtreeset_iter(&state->loop->loop_blocks, &iter); res == KEFIR_OK;
         res = kefir_hashtreeset_next(&iter)) {
        ASSIGN_DECL_CAST(kefir_opt_block_id_t, block_id, iter.entry);

        struct kefir_hashset_iterator block_iter;
        kefir_hashset_key_t key;
        for (res = kefir_hashset_iter(&state->control_flow.blocks[block_id].successors, &block_iter, &key);
             res == KEFIR_OK; res = kefir_hashset_next(&block_iter, &key)) {
            kefir_bool_t in_loop = kefir_hashtreeset_has(&state->loop->loop_blocks, (kefir_hashtreeset_entry_t) key);
            if (block_id == state->loop->loop_entry_block_id && exit_block_ref == KEFIR_ID_NONE && !in_loop) {
                exit_block_ref = key;
            } else {
                REQUIRE(in_loop, KEFIR_OK);
            }
        }
        if (res != KEFIR_ITERATOR_END) {
            REQUIRE_OK(res);
        }

        if (block_id != state->loop->loop_entry_block_id) {
            for (res = kefir_hashset_iter(&state->control_flow.blocks[block_id].predecessors, &block_iter, &key);
                 res == KEFIR_OK; res = kefir_hashset_next(&block_iter, &key)) {
                REQUIRE(kefir_hashtreeset_has(&state->loop->loop_blocks, (kefir_hashtreeset_entry_t) key), KEFIR_OK);
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }

        if (may_execute) {
            kefir_opt_instruction_ref_t instr_ref;
            for (res = kefir_opt_code_block_instr_head(&state->func->code, block_id, &instr_ref);
                 res == KEFIR_OK && instr_ref != KEFIR_ID_NONE;
                 res = kefir_opt_instruction_next_sibling(&state->func->code, instr_ref, &instr_ref)) {
                const struct kefir_opt_instruction *instr;
                REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, instr_ref, &instr));

                struct kefir_opt_instruction_use_iterator use_iter;
                for (res =
                         kefir_opt_code_container_instruction_use_instr_iter(&state->func->code, instr_ref, &use_iter);
                     res == KEFIR_OK; res = kefir_opt_code_container_instruction_use_next(&use_iter)) {
                    const struct kefir_opt_instruction *use_instr;
                    REQUIRE_OK(kefir_opt_code_container_instr(&state->func->code, use_iter.use_instr_ref, &use_instr));
                    REQUIRE(kefir_hashtreeset_has(&state->loop->loop_blocks,
                                                  (kefir_hashtreeset_entry_t) use_instr->block_id),
                            KEFIR_OK);
                }
                if (res != KEFIR_ITERATOR_END) {
                    REQUIRE_OK(res);
                }

                if (instr->operation.opcode == KEFIR_OPT_OPCODE_PHI ||
                    instr->operation.opcode == KEFIR_OPT_OPCODE_JUMP ||
                    instr->operation.opcode == KEFIR_OPT_OPCODE_BRANCH ||
                    instr->operation.opcode == KEFIR_OPT_OPCODE_BRANCH_COMPARE) {
                    // Intentionally left blank
                } else {
                    kefir_bool_t side_effect_free;
                    REQUIRE_OK(kefir_opt_instruction_is_side_effect_free(instr, &side_effect_free));
                    REQUIRE(side_effect_free, KEFIR_OK);
                }
            }
            if (res != KEFIR_ITERATOR_END) {
                REQUIRE_OK(res);
            }
        }
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    REQUIRE(exit_block_ref != KEFIR_ID_NONE, KEFIR_OK);

    REQUIRE_OK(kefir_opt_code_container_drop_control(&state->func->code, entry_block_tail_ref));
    REQUIRE_OK(kefir_opt_code_container_drop_instr(state->mem, &state->func->code, entry_block_tail_ref));
    REQUIRE_OK(kefir_opt_code_builder_finalize_jump(state->mem, &state->func->code, state->loop->loop_entry_block_id,
                                                    exit_block_ref, NULL));

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

static kefir_result_t loop_removal_impl(struct licm_state *state) {
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

static kefir_result_t loop_removal_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                         struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                         const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct licm_state state = {.mem = mem, .func = func};
    REQUIRE_OK(kefir_opt_code_control_flow_init(&state.control_flow));
    REQUIRE_OK(kefir_opt_code_loop_collection_init(&state.loops));

    kefir_result_t res = loop_removal_impl(&state);
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

const struct kefir_optimizer_pass KefirOptimizerPassLoopRemoval = {
    .name = "loop-removal", .apply = loop_removal_apply, .payload = NULL};

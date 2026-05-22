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
#include "kefir/optimizer/loop_nest.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

struct loop_rotate_state {
    struct kefir_mem *mem;
    struct kefir_opt_module *module;
    struct kefir_opt_function *func;
    struct kefir_opt_code_control_flow control_flow;
    struct kefir_opt_code_loop_collection loops;
};

static kefir_result_t try_rotate_nest(struct loop_rotate_state *state, struct kefir_opt_code_loop *loop) {
    for (struct kefir_opt_code_loop *nested = kefir_opt_code_loop_first_child(loop); nested != NULL;
         nested = kefir_opt_code_loop_next_sibling(nested)) {
        REQUIRE_OK(try_rotate_nest(state, nested));
    }

    if (loop->header_ref != state->func->code.entry_point && loop->header_ref != state->func->code.gate_block &&
        !kefir_hashset_has(&state->control_flow.indirect_jump_target_blocks, (kefir_hashset_key_t) loop->header_ref)) {
        REQUIRE_OK(kefir_opt_code_util_loop_try_rotate(state->mem, &state->func->code, loop, NULL));
    }
    return KEFIR_OK;
}

static kefir_result_t loop_rotate_impl(struct loop_rotate_state *state) {
    REQUIRE_OK(kefir_opt_code_control_flow_build(state->mem, &state->control_flow, &state->func->code));
    REQUIRE_OK(kefir_opt_code_loop_collection_build(state->mem, &state->loops, &state->control_flow));

    REQUIRE_OK(kefir_opt_code_util_insert_loop_preheaders(state->mem, &state->func->code, &state->control_flow,
                                                          &state->loops));

    REQUIRE_OK(kefir_opt_code_control_flow_reset(state->mem, &state->control_flow));
    REQUIRE_OK(kefir_opt_code_control_flow_build(state->mem, &state->control_flow, &state->func->code));

    kefir_result_t res;
    const struct kefir_opt_loop_nest *nest;
    struct kefir_opt_code_loop_nest_collection_iterator iter;
    for (res = kefir_opt_code_loop_nest_collection_iter(&state->loops, &nest, &iter); res == KEFIR_OK && nest != NULL;
         res = kefir_opt_code_loop_nest_collection_next(&nest, &iter)) {
        REQUIRE_OK(try_rotate_nest(state, kefir_opt_loop_nest_top(nest)));
    }
    if (res != KEFIR_ITERATOR_END) {
        REQUIRE_OK(res);
    }
    return KEFIR_OK;
}

static kefir_result_t loop_rotate_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                        struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                        const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct loop_rotate_state state = {.mem = mem, .func = func, .module = module};
    REQUIRE_OK(kefir_opt_code_control_flow_init(&state.control_flow));
    REQUIRE_OK(kefir_opt_code_loop_collection_init(&state.loops));

    kefir_result_t res = loop_rotate_impl(&state);

    kefir_opt_code_loop_collection_free(mem, &state.loops);
    kefir_opt_code_control_flow_free(mem, &state.control_flow);
    return res;
}

const struct kefir_optimizer_pass KefirOptimizerPassLoopRotate = {
    .name = "loop-rotate", .apply = loop_rotate_apply, .payload = NULL};

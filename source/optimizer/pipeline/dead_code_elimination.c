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

#include "kefir/optimizer/code.h"
#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/code_util.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/optimizer/liveness.h"

struct payload_param {
    struct kefir_mem *mem;
    struct kefir_opt_code_control_flow *control_flow;
    struct kefir_hashset *alive_instr;
};

static kefir_result_t is_block_alive(kefir_opt_block_id_t block_id, kefir_bool_t *alive_ptr, void *payload) {
    REQUIRE(alive_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    ASSIGN_DECL_CAST(struct payload_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code DCE parameter"));

    REQUIRE_OK(kefir_opt_code_control_flow_is_reachable_from_entry(param->control_flow, block_id, alive_ptr));
    return KEFIR_OK;
}

static kefir_result_t is_instruction_alive(kefir_opt_instruction_ref_t instr_ref, kefir_bool_t *alive_ptr,
                                           void *payload) {
    REQUIRE(alive_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    ASSIGN_DECL_CAST(struct payload_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code DCE parameter"));

    const struct kefir_opt_instruction *instr;
    REQUIRE_OK(kefir_opt_code_container_instr(param->control_flow->code, instr_ref, &instr));
    if (KEFIR_OPT_INSTRUCTION_IS_NONVOLATILE_LOAD(instr)) {
        struct kefir_opt_instruction_use_iterator use_iter;
        if (kefir_opt_code_container_instruction_use_instr_iter(param->control_flow->code, instr_ref, &use_iter) == KEFIR_ITERATOR_END) {
            *alive_ptr = false;
            return KEFIR_OK;
        }
    }

    *alive_ptr = kefir_hashset_has(param->alive_instr, (kefir_hashset_key_t) instr_ref);
    return KEFIR_OK;
}

static kefir_result_t is_block_predecessor(kefir_opt_block_id_t predecessor_block_id, kefir_opt_block_id_t block_id,
                                           kefir_bool_t *is_pred, void *payload) {
    REQUIRE(is_pred != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    ASSIGN_DECL_CAST(struct payload_param *, param, payload);
    REQUIRE(param != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code DCE parameter"));

    REQUIRE_OK(kefir_opt_code_control_flow_block_direct_predecessor(param->control_flow, predecessor_block_id, block_id,
                                                                    is_pred));
    return KEFIR_OK;
}

static kefir_result_t dead_code_elimination_impl(struct kefir_mem *mem, struct kefir_opt_function *func,
                                                 struct kefir_opt_code_control_flow *control_flow,
                                                 struct kefir_hashset *alive_instr) {
    REQUIRE_OK(kefir_opt_code_control_flow_build(mem, control_flow, &func->code));
    REQUIRE_OK(kefir_opt_code_liveness_collect_global(mem, &func->code, alive_instr));
    struct payload_param param = {.mem = mem, .control_flow = control_flow, .alive_instr = alive_instr};
    struct kefir_opt_code_container_dead_code_index index = {.is_block_alive = is_block_alive,
                                                             .is_instruction_alive = is_instruction_alive,
                                                             .is_block_predecessor = is_block_predecessor,
                                                             .payload = &param};
    REQUIRE_OK(kefir_opt_code_container_drop_dead_code(mem, &func->code, &index));
    return KEFIR_OK;
}

static kefir_result_t dead_code_elimination_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                                  struct kefir_opt_function *func,
                                                  const struct kefir_optimizer_pass *pass,
                                                  const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_control_flow control_flow;
    struct kefir_hashset alive_instr;
    REQUIRE_OK(kefir_opt_code_control_flow_init(&control_flow));
    REQUIRE_OK(kefir_hashset_init(&alive_instr, &kefir_hashtable_uint_ops));
    kefir_result_t res = dead_code_elimination_impl(mem, func, &control_flow, &alive_instr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_hashset_free(mem, &alive_instr);
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    res = kefir_hashset_free(mem, &alive_instr);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_control_flow_free(mem, &control_flow));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassDCE = {
    .name = "dead-code-elimination", .apply = dead_code_elimination_apply, .payload = NULL};

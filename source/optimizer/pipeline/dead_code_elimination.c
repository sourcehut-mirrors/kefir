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
#include "kefir/optimizer/analysis.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t is_block_alive(kefir_opt_block_id_t block_id, kefir_bool_t *alive_ptr, void *payload) {
    REQUIRE(alive_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_analysis *, analysis, payload);
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    REQUIRE_OK(kefir_opt_code_structure_is_reachable_from_entry(&analysis->structure, block_id, alive_ptr));
    return KEFIR_OK;
}

static kefir_result_t is_instruction_alive(kefir_opt_instruction_ref_t instr_ref, kefir_bool_t *alive_ptr,
                                           void *payload) {
    REQUIRE(alive_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_analysis *, analysis, payload);
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    REQUIRE_OK(kefir_opt_code_liveness_instruction_is_alive(&analysis->liveness, instr_ref, alive_ptr));
    return KEFIR_OK;
}

static kefir_result_t is_block_predecessor(kefir_opt_block_id_t predecessor_block_id, kefir_opt_block_id_t block_id,
                                           kefir_bool_t *is_pred, void *payload) {
    REQUIRE(is_pred != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean flag"));
    ASSIGN_DECL_CAST(struct kefir_opt_code_analysis *, analysis, payload);
    REQUIRE(analysis != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer code analysis"));

    REQUIRE_OK(kefir_opt_code_structure_block_direct_predecessor(&analysis->structure, predecessor_block_id, block_id,
                                                                 is_pred));
    return KEFIR_OK;
}

static kefir_result_t dead_code_elimination_impl(struct kefir_mem *mem, struct kefir_opt_function *func,
                                                 struct kefir_opt_code_analysis *analysis) {
    struct kefir_opt_code_container_dead_code_index index = {.is_block_alive = is_block_alive,
                                                             .is_instruction_alive = is_instruction_alive,
                                                             .is_block_predecessor = is_block_predecessor,
                                                             .payload = analysis};
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

    struct kefir_opt_code_analysis analysis;
    REQUIRE_OK(kefir_opt_code_analysis_init(&analysis));
    kefir_result_t res = kefir_opt_code_analyze(mem, &func->code, &analysis);
    REQUIRE_CHAIN(&res, dead_code_elimination_impl(mem, func, &analysis));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_analysis_free(mem, &analysis);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_analysis_free(mem, &analysis));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassDCE = {
    .name = "dead-code-elimination", .apply = dead_code_elimination_apply, .payload = NULL};

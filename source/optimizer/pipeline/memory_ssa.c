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

#include "kefir/core/basic-types.h"
#include "kefir/optimizer/pipeline.h"
#include "kefir/optimizer/control_flow.h"
#include "kefir/optimizer/memory_ssa.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

static kefir_result_t memory_ssa_apply(struct kefir_mem *mem, struct kefir_opt_module *module,
                                       struct kefir_opt_function *func, const struct kefir_optimizer_pass *pass,
                                       const struct kefir_optimizer_configuration *config) {
    UNUSED(pass);
    UNUSED(config);
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));

    struct kefir_opt_code_control_flow control_flow;
    struct kefir_opt_code_liveness liveness;
    struct kefir_opt_code_memssa memssa;
    REQUIRE_OK(kefir_opt_code_control_flow_init(&control_flow));
    REQUIRE_OK(kefir_opt_code_liveness_init(&liveness));
    REQUIRE_OK(kefir_opt_code_memssa_init(&memssa));

    kefir_result_t res = kefir_opt_code_control_flow_build(mem, &control_flow, &func->code);
    REQUIRE_CHAIN(&res, kefir_opt_code_liveness_build(mem, &liveness, &control_flow));
    REQUIRE_CHAIN(&res, kefir_opt_code_memssa_construct(mem, &memssa, &func->code, &control_flow, &liveness));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_memssa_free(mem, &memssa);
        kefir_opt_code_liveness_free(mem, &liveness);
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    res = kefir_opt_code_memssa_free(mem, &memssa);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_liveness_free(mem, &liveness);
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    res = kefir_opt_code_liveness_free(mem, &liveness);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_opt_code_control_flow_free(mem, &control_flow);
        return res;
    });
    REQUIRE_OK(kefir_opt_code_control_flow_free(mem, &control_flow));
    return KEFIR_OK;
}

const struct kefir_optimizer_pass KefirOptimizerPassMemorySSA = {
    .name = "memory-ssa", .apply = memory_ssa_apply, .payload = NULL};

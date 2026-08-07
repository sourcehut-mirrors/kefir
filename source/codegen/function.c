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

#include "kefir/codegen/function.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include "kefir/optimizer/linear_liveness.h"
#include "kefir/optimizer/schedule.h"
#include "kefir/optimizer/variable_scope.h"

kefir_result_t kefir_codegen_function_init(const struct kefir_opt_module *opt_module, struct kefir_opt_function *opt_func, struct kefir_codegen_function *func) {
    REQUIRE(opt_module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(opt_func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer function"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to codegen function"));

    REQUIRE_OK(kefir_opt_code_control_flow_init(&func->control_flow));
    REQUIRE_OK(kefir_opt_code_liveness_init(&func->liveness));
    REQUIRE_OK(kefir_opt_code_variable_scopes_init(&func->variable_scopes));
    REQUIRE_OK(kefir_codegen_local_variable_allocator_init(&func->variable_allocator));
    REQUIRE_OK(kefir_opt_code_schedule_init(&func->schedule));
    REQUIRE_OK(kefir_opt_code_linear_liveness_init(&func->linear_liveness));
    func->module = opt_module;
    func->function = opt_func;

    func->resolve_virtual_register = NULL;
    func->payload = NULL;
    return KEFIR_OK;
}

kefir_result_t kefir_codegen_function_free(struct kefir_mem *mem, struct kefir_codegen_function *func) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid codegen function"));

    REQUIRE_OK(kefir_opt_code_linear_liveness_free(mem, &func->linear_liveness));
    REQUIRE_OK(kefir_opt_code_schedule_free(mem, &func->schedule));
    REQUIRE_OK(kefir_codegen_local_variable_allocator_free(mem, &func->variable_allocator));
    REQUIRE_OK(kefir_opt_code_variable_scopes_free(mem, &func->variable_scopes));
    REQUIRE_OK(kefir_opt_code_liveness_free(mem, &func->liveness));
    REQUIRE_OK(kefir_opt_code_control_flow_free(mem, &func->control_flow));
    return KEFIR_OK;
}

/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#include "kefir/optimizer/function.h"
#include "kefir/optimizer/module.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_opt_function_init(const struct kefir_opt_module *module, const struct kefir_ir_function *ir_func,
                                       struct kefir_opt_function *func) {
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(ir_func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer function"));

    func->ir_func = ir_func;
    func->locals.type = ir_func->locals;
    func->locals.type_id = ir_func->locals_type_id;
    REQUIRE_OK(kefir_opt_code_container_init(&func->code));
    REQUIRE_OK(kefir_opt_code_debug_info_init(&func->debug_info));
    func->code.event_listener = &func->debug_info.listener;
    return KEFIR_OK;
}

kefir_result_t kefir_opt_function_free(struct kefir_mem *mem, struct kefir_opt_function *func) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer function"));

    REQUIRE_OK(kefir_opt_code_debug_info_free(mem, &func->debug_info));
    REQUIRE_OK(kefir_opt_code_container_free(mem, &func->code));
    func->ir_func = NULL;
    return KEFIR_OK;
}

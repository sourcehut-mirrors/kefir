/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

kefir_result_t kefir_opt_function_declaration_init(const struct kefir_opt_module *module,
                                                   const struct kefir_ir_function_decl *ir_decl,
                                                   struct kefir_opt_function_declaration *decl) {
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(ir_decl != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function declaration"));
    REQUIRE(decl != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer function declaration"));

    decl->ir_func_decl = ir_decl;
    REQUIRE_OK(kefir_opt_module_get_type(module, ir_decl->params_type_id, &decl->parameters_type));
    REQUIRE_OK(kefir_opt_module_get_type(module, ir_decl->result_type_id, &decl->result_type));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_function_init(const struct kefir_opt_module *module, const struct kefir_ir_function *ir_func,
                                       struct kefir_opt_function *func) {
    REQUIRE(module != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid optimizer module"));
    REQUIRE(ir_func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer function"));

    func->ir_func = ir_func;
    REQUIRE_OK(kefir_opt_module_get_function_declaration(module, ir_func->declaration->id, &func->declaration));
    REQUIRE_OK(kefir_opt_module_get_type(module, ir_func->locals_type_id, &func->locals_type));
    REQUIRE_OK(kefir_opt_code_container_init(&func->code));
    return KEFIR_OK;
}

kefir_result_t kefir_opt_function_free(struct kefir_mem *mem, struct kefir_opt_function *func) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(func != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to optimizer function"));

    REQUIRE_OK(kefir_opt_code_container_free(mem, &func->code));
    func->ir_func = NULL;
    func->declaration = NULL;
    func->locals_type = NULL;
    return KEFIR_OK;
}

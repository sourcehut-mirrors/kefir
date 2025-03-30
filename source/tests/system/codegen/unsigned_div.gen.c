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

#include <stdlib.h>
#include <stdio.h>
#include "kefir/ir/function.h"
#include "kefir/ir/builder.h"
#include "kefir/ir/module.h"
#include "kefir/core/mem.h"
#include "kefir/core/util.h"
#include "kefir/test/codegen.h"
#include "kefir/test/module_shim.h"

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_test_codegen codegen;
    kefir_test_codegen_init(mem, &codegen, stdout, NULL);

    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    kefir_id_t func_params, func_returns;
    struct kefir_ir_type *udiv_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *udiv_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(udiv_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(udiv_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *udiv_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "udiv", func_params, false, func_returns);
    REQUIRE(udiv_decl != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function *udiv_func = kefir_ir_module_new_function(mem, &module, udiv_decl, KEFIR_ID_NONE, 1024);
    REQUIRE(udiv_func != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, udiv_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));
    kefir_irbuilder_type_append(mem, udiv_func->declaration->params, KEFIR_IR_TYPE_LONG, 0, 0);
    kefir_irbuilder_type_append(mem, udiv_func->declaration->params, KEFIR_IR_TYPE_LONG, 0, 0);
    kefir_irbuilder_type_append(mem, udiv_func->declaration->result, KEFIR_IR_TYPE_LONG, 0, 0);
    kefir_irbuilder_block_appendi64(mem, &udiv_func->body, KEFIR_IR_OPCODE_UINT64_DIV, 0);
    kefir_irbuilder_block_appendi64(mem, &udiv_func->body, KEFIR_IR_OPCODE_RETURN, 0);

    struct kefir_ir_type *umod_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *umod_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(umod_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(umod_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *umod_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "umod", func_params, false, func_returns);
    REQUIRE(umod_decl != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function *umod_func = kefir_ir_module_new_function(mem, &module, umod_decl, KEFIR_ID_NONE, 1024);
    REQUIRE(umod_func != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, umod_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));
    kefir_irbuilder_type_append(mem, umod_func->declaration->params, KEFIR_IR_TYPE_LONG, 0, 0);
    kefir_irbuilder_type_append(mem, umod_func->declaration->params, KEFIR_IR_TYPE_LONG, 0, 0);
    kefir_irbuilder_type_append(mem, umod_func->declaration->result, KEFIR_IR_TYPE_LONG, 0, 0);
    kefir_irbuilder_block_appendi64(mem, &umod_func->body, KEFIR_IR_OPCODE_UINT64_MOD, 0);
    kefir_irbuilder_block_appendi64(mem, &umod_func->body, KEFIR_IR_OPCODE_RETURN, 0);

    KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module);

    KEFIR_CODEGEN_CLOSE(mem, &codegen.iface);
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

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
#include "kefir/ir/module.h"
#include "kefir/ir/function.h"
#include "kefir/ir/builder.h"
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
    struct kefir_ir_type *func1_decl_params = kefir_ir_module_new_type(mem, &module, 0, &func_params),
                         *func1_decl_result = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(func1_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(func1_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *func1_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "func1", func_params, false, func_returns);
    REQUIRE(func1_decl != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function *func1 = kefir_ir_module_new_function(mem, &module, func1_decl, KEFIR_ID_NONE, 1024);
    REQUIRE(func1 != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, func1_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    struct kefir_ir_type *func2_decl_params = kefir_ir_module_new_type(mem, &module, 3, &func_params),
                         *func2_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(func2_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(func2_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *func2_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "func2", func_params, false, func_returns);
    REQUIRE(func2_decl != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_external(mem, &module, func2_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    struct kefir_ir_type *func3_decl_params = kefir_ir_module_new_type(mem, &module, 3, &func_params),
                         *func3_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(func3_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(func3_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *func3_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "func3", func_params, false, func_returns);
    REQUIRE(func3_decl != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_external(mem, &module, func3_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    REQUIRE_OK(kefir_irbuilder_type_append(mem, func2_decl_params, KEFIR_IR_TYPE_INT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, func2_decl_params, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, func2_decl_params, KEFIR_IR_TYPE_INT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, func2_decl_result, KEFIR_IR_TYPE_INT, 0, 0));

    REQUIRE_OK(kefir_irbuilder_type_append(mem, func3_decl_params, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, func3_decl_params, KEFIR_IR_TYPE_BOOL, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, func3_decl_params, KEFIR_IR_TYPE_FLOAT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, func3_decl_result, KEFIR_IR_TYPE_FLOAT64, 0, 0));

    kefir_irbuilder_block_appendu64(mem, &func1->body, KEFIR_IR_OPCODE_INT_CONST, 0);
    kefir_irbuilder_block_appendf32(mem, &func1->body, KEFIR_IR_OPCODE_FLOAT32_CONST, 1.0f, 0.0f);
    kefir_irbuilder_block_appendu64(mem, &func1->body, KEFIR_IR_OPCODE_INT_CONST, 2);
    kefir_irbuilder_block_appendu64(mem, &func1->body, KEFIR_IR_OPCODE_INVOKE, func2_decl->id);
    kefir_irbuilder_block_appendf32(mem, &func1->body, KEFIR_IR_OPCODE_FLOAT32_CONST, 1.0f, 0.0f);
    kefir_irbuilder_block_appendu64(mem, &func1->body, KEFIR_IR_OPCODE_INT_CONST, 2);
    kefir_irbuilder_block_appendf32(mem, &func1->body, KEFIR_IR_OPCODE_FLOAT32_CONST, 3.0f, 0.0f);
    kefir_irbuilder_block_appendu64(mem, &func1->body, KEFIR_IR_OPCODE_INVOKE, func3_decl->id);

    REQUIRE_OK(KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module));
    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, &codegen.iface));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return KEFIR_OK;
}

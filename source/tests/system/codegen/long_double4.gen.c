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
#include "kefir/ir/module.h"
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

    kefir_id_t func_params, func_returns, locals_id;
    struct kefir_ir_type *sum_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *sum_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns),
                         *sum_locals = kefir_ir_module_new_type(mem, &module, 0, &locals_id);
    REQUIRE(sum_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(sum_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(sum_locals != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *sum_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "ldouble_sum", func_params, false, func_returns);
    REQUIRE(sum_decl != NULL, KEFIR_INTERNAL_ERROR);

    kefir_irbuilder_type_append(mem, sum_decl->params, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, sum_decl->params, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, sum_decl->result, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, sum_locals, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    struct kefir_ir_function *sum_func = kefir_ir_module_new_function_with_args(mem, &module, sum_decl, 1024);
    REQUIRE(sum_func != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, sum_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    kefir_irbuilder_block_appendi64(mem, &sum_func->body, KEFIR_IR_OPCODE_LONG_DOUBLE_ADD, 0);
    kefir_irbuilder_block_appendi64(mem, &sum_func->body, KEFIR_IR_OPCODE_RETURN, 0);

    struct kefir_ir_type *sub_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *sub_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns),
                         *sub_locals = kefir_ir_module_new_type(mem, &module, 0, &locals_id);
    REQUIRE(sub_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(sub_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(sub_locals != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *sub_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "ldouble_sub", func_params, false, func_returns);
    REQUIRE(sub_decl != NULL, KEFIR_INTERNAL_ERROR);

    kefir_irbuilder_type_append(mem, sub_decl->params, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, sub_decl->params, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, sub_decl->result, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, sub_locals, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    struct kefir_ir_function *sub_func = kefir_ir_module_new_function_with_args(mem, &module, sub_decl, 1024);
    REQUIRE(sub_func != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, sub_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    kefir_irbuilder_block_appendi64(mem, &sub_func->body, KEFIR_IR_OPCODE_LONG_DOUBLE_SUB, 0);
    kefir_irbuilder_block_appendi64(mem, &sub_func->body, KEFIR_IR_OPCODE_RETURN, 0);

    struct kefir_ir_type *mul_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *mul_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns),
                         *mul_locals = kefir_ir_module_new_type(mem, &module, 0, &locals_id);
    REQUIRE(mul_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(mul_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(mul_locals != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *mul_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "ldouble_mul", func_params, false, func_returns);
    REQUIRE(sum_decl != NULL, KEFIR_INTERNAL_ERROR);

    kefir_irbuilder_type_append(mem, mul_decl->params, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, mul_decl->params, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, mul_decl->result, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, mul_locals, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    struct kefir_ir_function *mul_func = kefir_ir_module_new_function_with_args(mem, &module, mul_decl, 1024);
    REQUIRE(mul_func != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, mul_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    kefir_irbuilder_block_appendi64(mem, &mul_func->body, KEFIR_IR_OPCODE_LONG_DOUBLE_MUL, 0);
    kefir_irbuilder_block_appendi64(mem, &mul_func->body, KEFIR_IR_OPCODE_RETURN, 0);

    struct kefir_ir_type *div_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *div_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns),
                         *div_locals = kefir_ir_module_new_type(mem, &module, 0, &locals_id);
    REQUIRE(div_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(div_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(div_locals != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *div_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "ldouble_div", func_params, false, func_returns);
    REQUIRE(sum_decl != NULL, KEFIR_INTERNAL_ERROR);

    kefir_irbuilder_type_append(mem, div_decl->params, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, div_decl->params, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, div_decl->result, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, div_locals, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    struct kefir_ir_function *div_func = kefir_ir_module_new_function_with_args(mem, &module, div_decl, 1024);
    REQUIRE(div_func != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, div_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    kefir_irbuilder_block_appendi64(mem, &div_func->body, KEFIR_IR_OPCODE_LONG_DOUBLE_DIV, 0);
    kefir_irbuilder_block_appendi64(mem, &div_func->body, KEFIR_IR_OPCODE_RETURN, 0);

    struct kefir_ir_type *neg_decl_params = kefir_ir_module_new_type(mem, &module, 1, &func_params),
                         *neg_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns),
                         *neg_locals = kefir_ir_module_new_type(mem, &module, 0, &locals_id);
    REQUIRE(neg_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(neg_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(neg_locals != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *neg_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "ldouble_neg", func_params, false, func_returns);
    REQUIRE(sum_decl != NULL, KEFIR_INTERNAL_ERROR);

    kefir_irbuilder_type_append(mem, neg_decl->params, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, neg_decl->result, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    kefir_irbuilder_type_append(mem, neg_locals, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0);
    struct kefir_ir_function *neg_func = kefir_ir_module_new_function_with_args(mem, &module, neg_decl, 1024);
    REQUIRE(neg_func != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, neg_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    kefir_irbuilder_block_appendi64(mem, &neg_func->body, KEFIR_IR_OPCODE_LONG_DOUBLE_NEG, 0);
    kefir_irbuilder_block_appendi64(mem, &neg_func->body, KEFIR_IR_OPCODE_RETURN, 0);

    KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module);

    KEFIR_CODEGEN_CLOSE(mem, &codegen.iface);
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

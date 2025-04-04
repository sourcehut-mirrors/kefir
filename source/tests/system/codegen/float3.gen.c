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

    kefir_id_t func_params, func_returns;
    struct kefir_ir_type *fequals_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *fequals_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(fequals_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(fequals_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *fequals_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "fequals", func_params, false, func_returns);
    REQUIRE(fequals_decl != NULL, KEFIR_INTERNAL_ERROR);
    kefir_irbuilder_type_append(mem, fequals_decl->params, KEFIR_IR_TYPE_FLOAT32, 0, 0);
    kefir_irbuilder_type_append(mem, fequals_decl->params, KEFIR_IR_TYPE_FLOAT32, 0, 0);
    kefir_irbuilder_type_append(mem, fequals_decl->result, KEFIR_IR_TYPE_BOOL, 0, 3);
    struct kefir_ir_function *fequals = kefir_ir_module_new_function_with_args(mem, &module, fequals_decl, 1024);
    REQUIRE(fequals != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, fequals_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));
    kefir_irbuilder_block_appendi64(mem, &fequals->body, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                    KEFIR_IR_COMPARE_FLOAT32_EQUALS);
    kefir_irbuilder_block_appendi64(mem, &fequals->body, KEFIR_IR_OPCODE_RETURN, 0);

    struct kefir_ir_type *fgreater_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *fgreater_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(fgreater_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(fgreater_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *fgreater_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "fgreater", func_params, false, func_returns);
    REQUIRE(fgreater_decl != NULL, KEFIR_INTERNAL_ERROR);
    kefir_irbuilder_type_append(mem, fgreater_decl->params, KEFIR_IR_TYPE_FLOAT32, 0, 0);
    kefir_irbuilder_type_append(mem, fgreater_decl->params, KEFIR_IR_TYPE_FLOAT32, 0, 0);
    kefir_irbuilder_type_append(mem, fgreater_decl->result, KEFIR_IR_TYPE_BOOL, 0, 3);
    struct kefir_ir_function *fgreater = kefir_ir_module_new_function_with_args(mem, &module, fgreater_decl, 1024);
    REQUIRE(fgreater != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, fgreater_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));
    kefir_irbuilder_block_appendi64(mem, &fgreater->body, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                    KEFIR_IR_COMPARE_FLOAT32_GREATER);
    kefir_irbuilder_block_appendi64(mem, &fgreater->body, KEFIR_IR_OPCODE_RETURN, 0);

    struct kefir_ir_type *flesser_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *flesser_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(flesser_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(flesser_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *flesser_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "flesser", func_params, false, func_returns);
    REQUIRE(flesser_decl != NULL, KEFIR_INTERNAL_ERROR);
    kefir_irbuilder_type_append(mem, flesser_decl->params, KEFIR_IR_TYPE_FLOAT32, 0, 0);
    kefir_irbuilder_type_append(mem, flesser_decl->params, KEFIR_IR_TYPE_FLOAT32, 0, 0);
    kefir_irbuilder_type_append(mem, flesser_decl->result, KEFIR_IR_TYPE_BOOL, 0, 3);
    struct kefir_ir_function *flesser = kefir_ir_module_new_function_with_args(mem, &module, flesser_decl, 1024);
    REQUIRE(flesser != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, flesser_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));
    kefir_irbuilder_block_appendi64(mem, &flesser->body, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                    KEFIR_IR_COMPARE_FLOAT32_LESSER);
    kefir_irbuilder_block_appendi64(mem, &flesser->body, KEFIR_IR_OPCODE_RETURN, 0);

    struct kefir_ir_type *dequals_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *dequals_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(dequals_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(dequals_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *dequals_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "dequals", func_params, false, func_returns);
    REQUIRE(dequals_decl != NULL, KEFIR_INTERNAL_ERROR);
    kefir_irbuilder_type_append(mem, dequals_decl->params, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, dequals_decl->params, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, dequals_decl->result, KEFIR_IR_TYPE_BOOL, 0, 3);
    struct kefir_ir_function *dequals = kefir_ir_module_new_function_with_args(mem, &module, dequals_decl, 1024);
    REQUIRE(dequals != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, dequals_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));
    kefir_irbuilder_block_appendi64(mem, &dequals->body, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                    KEFIR_IR_COMPARE_FLOAT64_EQUALS);
    kefir_irbuilder_block_appendi64(mem, &dequals->body, KEFIR_IR_OPCODE_RETURN, 0);

    struct kefir_ir_type *dgreater_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *dgreater_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(dgreater_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(dgreater_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *dgreater_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "dgreater", func_params, false, func_returns);
    REQUIRE(dgreater_decl != NULL, KEFIR_INTERNAL_ERROR);
    kefir_irbuilder_type_append(mem, dgreater_decl->params, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, dgreater_decl->params, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, dgreater_decl->result, KEFIR_IR_TYPE_BOOL, 0, 3);
    struct kefir_ir_function *dgreater = kefir_ir_module_new_function_with_args(mem, &module, dgreater_decl, 1024);
    REQUIRE(dgreater != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, dgreater_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));
    kefir_irbuilder_block_appendi64(mem, &dgreater->body, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                    KEFIR_IR_COMPARE_FLOAT64_GREATER);
    kefir_irbuilder_block_appendi64(mem, &dgreater->body, KEFIR_IR_OPCODE_RETURN, 0);

    struct kefir_ir_type *dlesser_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *dlesser_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(dlesser_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(dlesser_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *dlesser_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "dlesser", func_params, false, func_returns);
    REQUIRE(dlesser_decl != NULL, KEFIR_INTERNAL_ERROR);
    kefir_irbuilder_type_append(mem, dlesser_decl->params, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, dlesser_decl->params, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, dlesser_decl->result, KEFIR_IR_TYPE_BOOL, 0, 3);
    struct kefir_ir_function *dlesser = kefir_ir_module_new_function_with_args(mem, &module, dlesser_decl, 1024);
    REQUIRE(dlesser != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, dlesser_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));
    kefir_irbuilder_block_appendi64(mem, &dlesser->body, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                    KEFIR_IR_COMPARE_FLOAT64_LESSER);
    kefir_irbuilder_block_appendi64(mem, &dlesser->body, KEFIR_IR_OPCODE_RETURN, 0);

    KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module);
    KEFIR_CODEGEN_CLOSE(mem, &codegen.iface);
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

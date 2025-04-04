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
#include <string.h>
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
    struct kefir_ir_type *extractu_decl_params = kefir_ir_module_new_type(mem, &module, 0, &func_params),
                         *extractu_decl_result = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(extractu_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(extractu_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *extractu_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "extractu", func_params, false, func_returns);
    REQUIRE(extractu_decl != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, extractu_decl->name, KEFIR_IR_IDENTIFIER_FUNCTION));
    struct kefir_ir_function *extractu_func = kefir_ir_module_new_function_with_args(mem, &module, extractu_decl, 1024);
    REQUIRE(extractu_func != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 0, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 0, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 0, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 0, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 0, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 0, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 0, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 0, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 0, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 0, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(
        kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 0, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 1, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 2, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 4, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 8, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(
        kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 16, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(
        kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 32, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 1, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 2, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 4, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 8, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(
        kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_UNSIGNED, 16, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));

    struct kefir_ir_type *extracts_decl_params = kefir_ir_module_new_type(mem, &module, 0, &func_params),
                         *extracts_decl_result = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(extracts_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(extracts_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *extracts_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "extracts", func_params, false, func_returns);
    REQUIRE(extracts_decl != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, extracts_decl->name, KEFIR_IR_IDENTIFIER_FUNCTION));
    struct kefir_ir_function *extracts_func = kefir_ir_module_new_function_with_args(mem, &module, extracts_decl, 1024);
    REQUIRE(extracts_func != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 0, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 0, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 0, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 0, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 0, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 0, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 0, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 0, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 0, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 0, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 0, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 1, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 2, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 4, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 8, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 16, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 32, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 1, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 2, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 4, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 8, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IR_OPCODE_BITS_EXTRACT_SIGNED, 16, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));

    struct kefir_ir_type *insert_decl_params = kefir_ir_module_new_type(mem, &module, 0, &func_params),
                         *insert_decl_result = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(insert_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(insert_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *insert_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "insert", func_params, false, func_returns);
    REQUIRE(insert_decl != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, insert_decl->name, KEFIR_IR_IDENTIFIER_FUNCTION));
    struct kefir_ir_function *insert_func = kefir_ir_module_new_function_with_args(mem, &module, insert_decl, 1024);
    REQUIRE(insert_func != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 0, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 0, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 0, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 0, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 0, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 0, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 0, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 0, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 0, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 0, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 1, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 2, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 4, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 8, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 16, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 32, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 1, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 2, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 4, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 8, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IR_OPCODE_BITS_INSERT, 16, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IR_OPCODE_INT64_STORE, 0));

    REQUIRE_OK(KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module));
    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, &codegen.iface));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return KEFIR_OK;
}

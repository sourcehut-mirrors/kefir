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
    struct kefir_ir_type *decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *decl_result = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *decl =
        kefir_ir_module_new_function_declaration(mem, &module, "scope_test", func_params, false, func_returns);
    REQUIRE(decl != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function *func = kefir_ir_module_new_function(mem, &module, decl, 1024);
    REQUIRE(func != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));
    kefir_irbuilder_type_append(mem, func->declaration->params, KEFIR_IR_TYPE_LONG, 0, 0);
    kefir_irbuilder_type_append(mem, func->declaration->params, KEFIR_IR_TYPE_LONG, 0, 0);
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_PICK, 0);      // [C, SZ, SZ]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_SCOPE_PUSH, 0);       // [C, SZ, SZ, S1]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1);  // [C, SZ, S1, SZ]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT_CONST, 0);        // [C, SZ, S1, SZ, 0]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_STACK_ALLOC, 0);      // [C, SZ, S1, P1]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_POP, 0);       // [C, SZ, S1]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_SCOPE_POP, 0);        // [C, SZ]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1);  // [SZ, C]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT_CONST, -1);       // [SZ, C-1]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT64_ADD, 0);        // [SZ, C-1]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1);  // [C-1, SZ]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_PICK, 1);      // [C-1, SZ, C-1]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT_CONST, 0);        // [C-1, SZ, C-1, 0]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                    KEFIR_IR_COMPARE_INT64_EQUALS);                        // [C-1, SZ, C-1==0]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT64_BOOL_NOT, 0);  // [C-1, SZ, C-1!=0]
    kefir_irbuilder_block_appendu64_2(mem, &func->body, KEFIR_IR_OPCODE_BRANCH, 0,
                                      KEFIR_IR_BRANCH_CONDITION_8BIT);  // [C-1, SZ]

    KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module);

    KEFIR_CODEGEN_CLOSE(mem, &codegen.iface);
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

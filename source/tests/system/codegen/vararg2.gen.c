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
#include "kefir/ir/builtins.h"
#include "kefir/core/mem.h"
#include "kefir/core/util.h"
#include "kefir/test/codegen.h"
#include "kefir/test/module_shim.h"

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_test_codegen codegen;
    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    kefir_id_t locals_id, inttype_id;
    kefir_id_t func_returns;
    struct kefir_ir_type *inttype = kefir_ir_module_new_type(mem, &module, 1, &inttype_id),
                         *sumint_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns),
                         *sumint_locals = kefir_ir_module_new_type(mem, &module, 1, &locals_id);
    REQUIRE(inttype != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(sumint_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *sumint_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "sumint", inttype_id, true, func_returns);
    REQUIRE(sumint_decl != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function *sumint = kefir_ir_module_new_function(mem, &module, sumint_decl, locals_id, 1024);
    REQUIRE(sumint != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, sumint_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    REQUIRE_OK(kefir_test_codegen_init(mem, &codegen, stdout, NULL));

    REQUIRE_OK(kefir_irbuilder_type_append(mem, inttype, KEFIR_IR_TYPE_INT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, sumint_decl_result, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, sumint_locals, KEFIR_IR_TYPE_BUILTIN, 0, KEFIR_IR_TYPE_BUILTIN_VARARG));
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_NOP, 0);                   // 0: [C, L*]
    kefir_irbuilder_block_appendu32(mem, &sumint->body, KEFIR_IR_OPCODE_GET_LOCAL, locals_id, 0);  // 1: [C, V*]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_VARARG_START, 0);          // 2: [C]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_INT_CONST, 0);             // 3: [C, S=0]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_VSTACK_PICK, 1);           // 4: [C, S, C]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_INT_CONST, 0);             // 5: [C, S, C, 0]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                    KEFIR_IR_COMPARE_INT64_EQUALS);  // 6: [C, S, C==0]
    kefir_irbuilder_block_appendu64_2(mem, &sumint->body, KEFIR_IR_OPCODE_BRANCH, 17,
                                      KEFIR_IR_BRANCH_CONDITION_8BIT);                               // 7: [C, S] -> @16
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_NOP, 0);                     // 8: [C, S, L*]
    kefir_irbuilder_block_appendu32(mem, &sumint->body, KEFIR_IR_OPCODE_GET_LOCAL, locals_id, 0);    // 9: [C, S, V*]
    kefir_irbuilder_block_appendu32(mem, &sumint->body, KEFIR_IR_OPCODE_VARARG_GET, inttype_id, 0);  // 10: [C, S, A]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_INT64_ADD, 0);               // 11: [C, S+A]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1);         // 12: [S+A, C]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_INT_CONST, -1);              // 13: [S+A, C-1]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_INT64_ADD, 0);               // 14: [S+A, C-1]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1);         // 15: [C-1, S+A]
    kefir_irbuilder_block_appendu64(mem, &sumint->body, KEFIR_IR_OPCODE_JUMP, 4);  // 16: [C-1, S+A] -> @4
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_NOP, 0);   // 17: [0, S, L*]
    kefir_irbuilder_block_appendu32(mem, &sumint->body, KEFIR_IR_OPCODE_GET_LOCAL, locals_id, 0);  // 18: [0, S, V*]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_VARARG_END, 0);            // 19: [0, S]
    kefir_irbuilder_block_appendi64(mem, &sumint->body, KEFIR_IR_OPCODE_RETURN, 0);                // 20: [0, S]

    REQUIRE_OK(KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module));
    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, &codegen.iface));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

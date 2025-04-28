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
#include "vararg.inc.c"

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_test_codegen codegen;
    kefir_test_codegen_init(mem, &codegen, stdout, NULL);

    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    kefir_id_t locals_id, ldouble_type_id;
    kefir_id_t func_params, func_returns;
    struct kefir_ir_type *ldouble_type = kefir_ir_module_new_type(mem, &module, 1, &ldouble_type_id),
                         *sumldouble_decl_params = kefir_ir_module_new_type(mem, &module, 1, &func_params),
                         *sumldouble_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns),
                         *sumldouble_locals = kefir_ir_module_new_type(mem, &module, 2, &locals_id);
    REQUIRE(ldouble_type != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(sumldouble_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *sumldouble_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "sumldouble", func_params, true, func_returns);
    REQUIRE(sumldouble_decl != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(kefir_irbuilder_type_append(mem, ldouble_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, sumldouble_decl_params, KEFIR_IR_TYPE_INT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, sumldouble_decl_result, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(generate_va_list_type(mem, sumldouble_locals));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, sumldouble_locals, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    struct kefir_ir_function *sumldouble = kefir_ir_module_new_function_with_args(mem, &module, sumldouble_decl, 1024);
    REQUIRE(sumldouble != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, sumldouble_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    kefir_irbuilder_block_appendu32_4(mem, &sumldouble->body, KEFIR_IR_OPCODE_GET_LOCAL, locals_id, 0, locals_id,
                                      0);                                                      // 0: [C, V*]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IR_OPCODE_VARARG_START, 0);  // 1: [C]
    kefir_irbuilder_block_appendu32_4(mem, &sumldouble->body, KEFIR_IR_OPCODE_GET_LOCAL, locals_id, 1, locals_id,
                                      1);  // 2: [C, LD*]
    kefir_irbuilder_block_append_long_double(mem, &sumldouble->body, KEFIR_IR_OPCODE_LONG_DOUBLE_CONST,
                                             1.0L);                                                 // 3: [C, LD*, LD]
    kefir_irbuilder_block_appendu64(mem, &sumldouble->body, KEFIR_IR_OPCODE_LONG_DOUBLE_STORE, 0);  // 4: [C]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IR_OPCODE_VSTACK_PICK, 0);        // 5: [C, C]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IR_OPCODE_INT_CONST, 0);          // 6: [C, C, 0]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IR_OPCODE_SCALAR_COMPARE,
                                    KEFIR_IR_COMPARE_INT64_EQUALS);  // 7: [C, C==0]
    kefir_irbuilder_block_appendu64_2(mem, &sumldouble->body, KEFIR_IR_OPCODE_BRANCH, 21,
                                      KEFIR_IR_BRANCH_CONDITION_8BIT);  // 8: [C] -> @21
    kefir_irbuilder_block_appendu32_4(mem, &sumldouble->body, KEFIR_IR_OPCODE_GET_LOCAL, locals_id, 0, locals_id,
                                      0);  // 9: [C, V*]
    kefir_irbuilder_block_appendu64(mem, &sumldouble->body, KEFIR_IR_OPCODE_NULL_REF, 0);
    kefir_irbuilder_block_appendu32(mem, &sumldouble->body, KEFIR_IR_OPCODE_VARARG_GET, ldouble_type_id,
                                    0);  // 10: [C, ARG*]
    kefir_irbuilder_block_appendu32_4(mem, &sumldouble->body, KEFIR_IR_OPCODE_GET_LOCAL, locals_id, 1, locals_id,
                                      1);                                                         // 12: [C, LD1, LD*]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IR_OPCODE_LONG_DOUBLE_ADD, 0);  // 14: [C, LD]
    kefir_irbuilder_block_appendu32_4(mem, &sumldouble->body, KEFIR_IR_OPCODE_GET_LOCAL, locals_id, 1, locals_id,
                                      1);                                                           // 15: [C, LD, LD*]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1);    // 16: [C, LD*, LD]
    kefir_irbuilder_block_appendu64(mem, &sumldouble->body, KEFIR_IR_OPCODE_LONG_DOUBLE_STORE, 0);  // 17: [C]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IR_OPCODE_INT_CONST, -1);         // 18: [C-1]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IR_OPCODE_INT64_ADD, 0);          // 19: [C-1]
    kefir_irbuilder_block_appendu64(mem, &sumldouble->body, KEFIR_IR_OPCODE_JUMP, 6);               // 20: [C-1] -> @5
    kefir_irbuilder_block_appendu32_4(mem, &sumldouble->body, KEFIR_IR_OPCODE_GET_LOCAL, locals_id, 0, locals_id,
                                      0);                                                    // 21: [C, V*]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IR_OPCODE_VARARG_END, 0);  // 22: [C]
    kefir_irbuilder_block_appendu32_4(mem, &sumldouble->body, KEFIR_IR_OPCODE_GET_LOCAL, locals_id, 1, locals_id,
                                      1);  // 23: [C, LD*]
    kefir_irbuilder_block_appendu64(mem, &sumldouble->body, KEFIR_IR_OPCODE_RETURN, 0);

    REQUIRE_OK(KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module));
    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, &codegen.iface));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

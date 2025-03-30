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
    struct kefir_ir_type *decl_params = kefir_ir_module_new_type(mem, &module, 0, &func_params),
                         *decl_result = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *decl =
        kefir_ir_module_new_function_declaration(mem, &module, "func1", func_params, false, func_returns);
    REQUIRE(decl != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, decl->name, KEFIR_IR_IDENTIFIER_FUNCTION));
    struct kefir_ir_function *func = kefir_ir_module_new_function(mem, &module, decl, KEFIR_ID_NONE, 1024);
    REQUIRE(func != NULL, KEFIR_INTERNAL_ERROR);

    kefir_id_t literal_id;
    const char *literal = "Hello, world!";
    REQUIRE_OK(kefir_ir_module_string_literal(mem, &module, KEFIR_IR_STRING_LITERAL_MULTIBYTE, true, literal,
                                              strlen(literal) + 1, &literal_id));
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_STRING_REF, literal_id);
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT64_STORE, 1));

    literal = "Hello,\ncruel\tworld!";
    REQUIRE_OK(kefir_ir_module_string_literal(mem, &module, KEFIR_IR_STRING_LITERAL_MULTIBYTE, true, literal,
                                              strlen(literal) + 1, &literal_id));
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_STRING_REF, literal_id);
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT64_STORE, 1));

    literal = "\n\n\n\t\t\t   \t\t\t\v\v\v\n";
    REQUIRE_OK(kefir_ir_module_string_literal(mem, &module, KEFIR_IR_STRING_LITERAL_MULTIBYTE, true, literal,
                                              strlen(literal) + 1, &literal_id));
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_STRING_REF, literal_id);
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT64_STORE, 1));

    literal = "\'\"Hey ho!\"\'\\\"";
    REQUIRE_OK(kefir_ir_module_string_literal(mem, &module, KEFIR_IR_STRING_LITERAL_MULTIBYTE, true, literal,
                                              strlen(literal) + 1, &literal_id));
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_STRING_REF, literal_id);
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT64_STORE, 1));

    literal = "\0\0\0\n\0";
    REQUIRE_OK(
        kefir_ir_module_string_literal(mem, &module, KEFIR_IR_STRING_LITERAL_MULTIBYTE, true, literal, 6, &literal_id));
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_STRING_REF, literal_id);
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT64_STORE, 1));

    REQUIRE_OK(KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module));
    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, &codegen.iface));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return KEFIR_OK;
}

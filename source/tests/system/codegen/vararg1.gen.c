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
#include "kefir/ir/builder.h"
#include "kefir/ir/module.h"
#include "kefir/core/mem.h"
#include "kefir/core/util.h"
#include "kefir/test/codegen.h"
#include "kefir/test/module_shim.h"

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    const char *FMT = "INTEGER: %i\n";

    struct kefir_test_codegen codegen;
    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    kefir_id_t func_params, func_returns;
    struct kefir_ir_type *printint_decl_params = kefir_ir_module_new_type(mem, &module, 1, &func_params),
                         *printint_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(printint_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(printint_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *printint_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "printint", func_params, false, func_returns);
    REQUIRE(printint_decl != NULL, KEFIR_INTERNAL_ERROR);

    struct kefir_ir_type *sprintf_decl_params = kefir_ir_module_new_type(mem, &module, 3, &func_params),
                         *sprintf_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(sprintf_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(sprintf_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *sprintf_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "sprintf", func_params, true, func_returns);
    REQUIRE(sprintf_decl != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_external(mem, &module, "sprintf", KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    kefir_id_t fmt_id, fmt_type_id;
    struct kefir_ir_type *string_type = kefir_ir_module_new_type(mem, &module, 2, &fmt_type_id);
    REQUIRE_OK(kefir_irbuilder_type_append(mem, string_type, KEFIR_IR_TYPE_ARRAY, 0, strlen(FMT) + 1));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, string_type, KEFIR_IR_TYPE_CHAR, 0, 0));
    struct kefir_ir_data *fmt_data1 = kefir_ir_module_new_named_data(
        mem, &module, kefir_ir_module_symbol(mem, &module, "fmt", &fmt_id), KEFIR_IR_DATA_GLOBAL_STORAGE, fmt_type_id);
    REQUIRE_OK(kefir_ir_module_declare_local(mem, &module, "fmt", KEFIR_IR_IDENTIFIER_GLOBAL_DATA));
    REQUIRE_OK(kefir_ir_data_set_string(mem, fmt_data1, 0, KEFIR_IR_STRING_LITERAL_MULTIBYTE, FMT, strlen(FMT)));
    REQUIRE_OK(kefir_ir_data_finalize(mem, fmt_data1));

    kefir_id_t result_id, result_type_id;
    struct kefir_ir_type *string2_type = kefir_ir_module_new_type(mem, &module, 2, &result_type_id);
    REQUIRE_OK(kefir_irbuilder_type_append(mem, string2_type, KEFIR_IR_TYPE_ARRAY, 0, 256));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, string2_type, KEFIR_IR_TYPE_CHAR, 0, 0));
    REQUIRE_OK(kefir_ir_data_finalize(
        mem, kefir_ir_module_new_named_data(mem, &module, kefir_ir_module_symbol(mem, &module, "result", &result_id),
                                            KEFIR_IR_DATA_GLOBAL_STORAGE, result_type_id)));
    REQUIRE_OK(kefir_ir_module_declare_local(mem, &module, "result", KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    REQUIRE_OK(kefir_test_codegen_init(mem, &codegen, stdout, NULL));

    REQUIRE_OK(kefir_irbuilder_type_append(mem, printint_decl_params, KEFIR_IR_TYPE_INT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, printint_decl_result, KEFIR_IR_TYPE_WORD, 0, 0));
    struct kefir_ir_function *printint = kefir_ir_module_new_function_with_args(mem, &module, printint_decl, 1024);
    REQUIRE(printint != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, printint_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));
    kefir_irbuilder_block_appendu64(mem, &printint->body, KEFIR_IR_OPCODE_GET_GLOBAL, result_id);  // 0: [I, R*]
    kefir_irbuilder_block_appendi64(mem, &printint->body, KEFIR_IR_OPCODE_VSTACK_PICK, 0);         // 1: [I, R*, R*]
    kefir_irbuilder_block_appendu64(mem, &printint->body, KEFIR_IR_OPCODE_GET_GLOBAL, fmt_id);     // 2: [I, R*, R*, F*]
    kefir_irbuilder_block_appendi64(mem, &printint->body, KEFIR_IR_OPCODE_VSTACK_PICK, 3);  // 3: [I, R*, R*, F*, I]
    kefir_irbuilder_block_appendu64(mem, &printint->body, KEFIR_IR_OPCODE_INVOKE, sprintf_decl->id);  // 4: [I, R*, O]
    kefir_irbuilder_block_appendi64(mem, &printint->body, KEFIR_IR_OPCODE_VSTACK_POP, 0);             // 5: [I, R*]
    kefir_irbuilder_block_appendi64(mem, &printint->body, KEFIR_IR_OPCODE_RETURN, 0);                 // 5: [I, R*]

    REQUIRE_OK(kefir_irbuilder_type_append(mem, sprintf_decl_params, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, sprintf_decl_params, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, sprintf_decl_params, KEFIR_IR_TYPE_INT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, sprintf_decl_result, KEFIR_IR_TYPE_INT, 0, 0));

    REQUIRE_OK(KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module));
    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, &codegen.iface));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

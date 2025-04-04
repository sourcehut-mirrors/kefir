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
    kefir_id_t type_id;
    kefir_id_t func_returns;
    struct kefir_ir_type *decl_params = kefir_ir_module_new_type(mem, &module, 10, &type_id),
                         *decl_result = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *decl =
        kefir_ir_module_new_function_declaration(mem, &module, "func1", type_id, false, func_returns);
    REQUIRE(decl != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl->params, KEFIR_IR_TYPE_STRUCT, 0, 5));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl->params, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl->params, KEFIR_IR_TYPE_WORD, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl->params, KEFIR_IR_TYPE_CHAR, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl->params, KEFIR_IR_TYPE_BITS, 4, 6));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl->params, KEFIR_IR_TYPE_UNION, 0, 2));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl->params, KEFIR_IR_TYPE_STRUCT, 0, 2));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl->params, KEFIR_IR_TYPE_SHORT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl->params, KEFIR_IR_TYPE_SHORT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl->params, KEFIR_IR_TYPE_INT, 0, 0));

    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, decl->name, KEFIR_IR_IDENTIFIER_FUNCTION));
    struct kefir_ir_function *func = kefir_ir_module_new_function_with_args(mem, &module, decl, 0);
    REQUIRE(func != NULL, KEFIR_INTERNAL_ERROR);

    for (kefir_size_t i = 0; i < kefir_ir_type_children(decl_params); i++) {
        REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT_CONST, 0));
        REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1));
        REQUIRE_OK(kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT64_STORE, 1));
    }

    struct kefir_ir_data *data1 =
        kefir_ir_module_new_named_data(mem, &module, "DataX", KEFIR_IR_DATA_GLOBAL_STORAGE, type_id);
    REQUIRE(data1 != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_data_finalize(mem, data1));
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, "DataX", KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    REQUIRE_OK(KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module));

    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, &codegen.iface));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return KEFIR_OK;
}

/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/test/codegen.h"

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_test_codegen codegen;
    kefir_test_codegen_init(mem, &codegen, stdout, NULL);

    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    kefir_id_t type1_id;
    struct kefir_ir_type *type1 = kefir_ir_module_new_type(mem, &module, 5, &type1_id);
    REQUIRE(type1 != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_STRUCT, 0, 4));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT8, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT16, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT64, 0, 0));

    kefir_id_t func_params, func_returns;
    struct kefir_ir_type *trunc_decl_params = kefir_ir_module_new_type(mem, &module, 2, &func_params),
                         *trunc_decl_result = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(trunc_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(trunc_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *trunc_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "custom_truncate", func_params, false, func_returns);
    REQUIRE(trunc_decl != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function *trunc = kefir_ir_module_new_function(mem, &module, trunc_decl, KEFIR_ID_NONE, 1024);
    REQUIRE(trunc != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, trunc_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL));
    kefir_irbuilder_type_append(mem, trunc->declaration->params, KEFIR_IR_TYPE_WORD, 0, 0);
    kefir_irbuilder_type_append(mem, trunc->declaration->params, KEFIR_IR_TYPE_LONG, 0, 0);

    struct kefir_abi_amd64_type_layout type_layout;
    REQUIRE_OK(kefir_abi_amd64_type_layout(mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V, type1, &type_layout));

    ASSIGN_DECL_CAST(const struct kefir_abi_amd64_typeentry_layout *, entry_layout,
                     kefir_vector_at(&type_layout.layout, 1));

    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_PICK, 0);  // 0: [S*, V, V]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_PICK, 2);  // 0: [S*, V, V, S*]
    kefir_irbuilder_block_appendu64(mem, &trunc->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);                // 0: [S*, V, V, I8*]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_XCHG, 1);    // 0: [S*, V, I8*, V]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_STORE8, 0);  // 0: [S*, V]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_PICK, 0);    // 0: [S*, V, V]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_PICK, 2);    // 0: [S*, V, V, S*]
    entry_layout = kefir_vector_at(&type_layout.layout, 2);
    kefir_irbuilder_block_appendu64(mem, &trunc->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);                 // 0: [S*, V, V, I16*]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_XCHG, 1);     // 0: [S*, V, I16*, V]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_STORE16, 0);  // 0: [S*, V]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_PICK, 0);     // 0: [S*, V, V]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_PICK, 2);     // 0: [S*, V, V, S*]
    entry_layout = kefir_vector_at(&type_layout.layout, 3);
    kefir_irbuilder_block_appendu64(mem, &trunc->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);                 // 0: [S*, V, V, I32*]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_XCHG, 1);     // 0: [S*, V, I32*, V]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_STORE32, 0);  // 0: [S*, V]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_PICK, 0);     // 0: [S*, V, V]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_PICK, 2);     // 0: [S*, V, V, S*]
    entry_layout = kefir_vector_at(&type_layout.layout, 4);
    kefir_irbuilder_block_appendu64(mem, &trunc->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);                 // 0: [S*, V, V, I64*]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_XCHG, 1);     // 0: [S*, V, I64*, V]
    kefir_irbuilder_block_appendi64(mem, &trunc->body, KEFIR_IROPCODE_STORE64, 0);  // 0: [S*, V]

    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &type_layout));
    KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module);
    KEFIR_CODEGEN_CLOSE(mem, &codegen.iface);
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

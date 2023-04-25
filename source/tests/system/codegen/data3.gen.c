/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
#include "kefir/codegen/system-v-amd64.h"
#include "kefir/target/abi/system-v-amd64/data_layout.h"

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_codegen_amd64 codegen;
    kefir_codegen_sysv_amd64_init(mem, &codegen, stdout, NULL);

    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    kefir_id_t type1_id;
    struct kefir_ir_type *type1 = kefir_ir_module_new_type(mem, &module, 9, &type1_id);
    REQUIRE(type1 != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_STRUCT, 0, 8));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT8, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT8, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT16, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT16, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT32, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT64, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, type1, KEFIR_IR_TYPE_INT64, 0, 0));

    kefir_id_t func_params, func_returns;
    struct kefir_ir_type *sum_decl_params = kefir_ir_module_new_type(mem, &module, 1, &func_params),
                         *sum_decl_result = kefir_ir_module_new_type(mem, &module, 1, &func_returns);
    REQUIRE(sum_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(sum_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *sum_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "sum", func_params, false, func_returns);
    REQUIRE(sum_decl != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function *sum = kefir_ir_module_new_function(mem, &module, sum_decl, KEFIR_ID_NONE, 1024);
    REQUIRE(sum != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, sum_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL));
    kefir_irbuilder_type_append(mem, sum->declaration->params, KEFIR_IR_TYPE_WORD, 0, 0);
    kefir_irbuilder_type_append(mem, sum->declaration->result, KEFIR_IR_TYPE_LONG, 0, 0);

    struct kefir_abi_sysv_amd64_type_layout type_layout;
    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout(type1, mem, &type_layout));

    ASSIGN_DECL_CAST(const struct kefir_abi_sysv_amd64_typeentry_layout *, entry_layout,
                     kefir_vector_at(&type_layout.layout, 1));

    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_PUSHI64, 0);  // 0: [S*, 0]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_PICK, 1);     // 1: [S*, 0, S*]
    kefir_irbuilder_block_appendu64(mem, &sum->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);              // 2: [S*, 0, U8*]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_LOAD8U, 0);  // 3: [S*, 0, U8]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_IADD, 0);    // 4: [S*, U8]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_PICK, 1);    // 1: [S*, U8, S*]
    entry_layout = kefir_vector_at(&type_layout.layout, 2);
    kefir_irbuilder_block_appendu64(mem, &sum->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);              // 2: [S*, U8, I8*]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_LOAD8I, 0);  // 3: [S*, U8, I8]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_IADD, 0);    // 4: [S*, U8+I8]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_PICK, 1);    // 1: [S*, U8+I8, S*]
    entry_layout = kefir_vector_at(&type_layout.layout, 3);
    kefir_irbuilder_block_appendu64(mem, &sum->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);               // 2: [S*, U8+I8, U16*]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_LOAD16U, 0);  // 3: [S*, U8+I8, U16]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_IADD, 0);     // 4: [S*, U8+I8+U16]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_PICK, 1);     // 1: [S*, U8+I8+U16, S*]
    entry_layout = kefir_vector_at(&type_layout.layout, 4);
    kefir_irbuilder_block_appendu64(mem, &sum->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);               // 2: [S*, U8+I8+U16, I16*]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_LOAD16I, 0);  // 3: [S*, U8+I8+U16, I16]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_IADD, 0);     // 4: [S*, U8+I8+U16+I16]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_PICK, 1);     // 1: [S*, U8+I8+U16+I16, S*]
    entry_layout = kefir_vector_at(&type_layout.layout, 5);
    kefir_irbuilder_block_appendu64(mem, &sum->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);               // 2: [S*, U8+I8+U16+I16, U32*]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_LOAD32U, 0);  // 3: [S*, U8+I8+U16+I16, U32]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_IADD, 0);     // 4: [S*, U8+I8+U16+I16+U32]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_PICK, 1);     // 1: [S*, U8+I8+U16+I16+U32, S*]
    entry_layout = kefir_vector_at(&type_layout.layout, 6);
    kefir_irbuilder_block_appendu64(mem, &sum->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);               // 2: [S*, U8+I8+U16+I16+U32, I32*]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_LOAD32I, 0);  // 3: [S*, U8+I8+U16+I16+U32, I32]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_IADD, 0);     // 4: [S*, U8+I8+U16+I16+U32+I32]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_PICK, 1);     // 1: [S*, U8+I8+U16+I16+U32+I32, S*]
    entry_layout = kefir_vector_at(&type_layout.layout, 7);
    kefir_irbuilder_block_appendu64(mem, &sum->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);              // 2: [S*, U8+I8+U16+I16+U32+I32, I64*]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_LOAD64, 0);  // 3: [S*, U8+I8+U16+I16+U32+I32, I64]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_IADD, 0);    // 4: [S*, U8+I8+U16+I16+U32+I32+I64]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_PICK, 1);    // 1: [S*, SUM, S*]
    entry_layout = kefir_vector_at(&type_layout.layout, 8);
    kefir_irbuilder_block_appendu64(mem, &sum->body, KEFIR_IROPCODE_IADD1,
                                    entry_layout->relative_offset);               // 2: [S*, VAL, SUM*]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_PICK, 0);     // 1: [S*, VAL, SUM*, SUM*]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_XCHG, 2);     // 1: [S*, SUM*, SUM*, VAL]
    kefir_irbuilder_block_appendi64(mem, &sum->body, KEFIR_IROPCODE_STORE64, 0);  // 1: [S*, SUM*]

    REQUIRE_OK(kefir_abi_sysv_amd64_type_layout_free(mem, &type_layout));
    KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module);
    KEFIR_CODEGEN_CLOSE(mem, &codegen.iface);
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

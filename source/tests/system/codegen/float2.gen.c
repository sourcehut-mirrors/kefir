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
#include "kefir/target/abi/amd64/type_layout.h"
#include "kefir/test/codegen.h"
#include "kefir/test/module_shim.h"

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_test_codegen codegen;
    kefir_test_codegen_init(mem, &codegen, stdout, NULL);

    kefir_id_t locals_id;
    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));
    kefir_id_t func_params, func_returns;
    struct kefir_ir_type *decl_params = kefir_ir_module_new_type(mem, &module, 1, &func_params),
                         *decl_result = kefir_ir_module_new_type(mem, &module, 4, &func_returns),
                         *func_locals = kefir_ir_module_new_type(mem, &module, 4, &locals_id);
    REQUIRE(decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *decl =
        kefir_ir_module_new_function_declaration(mem, &module, "circle", func_params, false, func_returns);
    REQUIRE(decl != NULL, KEFIR_INTERNAL_ERROR);
    kefir_irbuilder_type_append(mem, decl->params, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, decl->result, KEFIR_IR_TYPE_STRUCT, 0, 3);
    kefir_irbuilder_type_append(mem, decl->result, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, decl->result, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, decl->result, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, func_locals, KEFIR_IR_TYPE_STRUCT, 0, 3);
    kefir_irbuilder_type_append(mem, func_locals, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, func_locals, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    kefir_irbuilder_type_append(mem, func_locals, KEFIR_IR_TYPE_FLOAT64, 0, 0);
    struct kefir_ir_function *func = kefir_ir_module_new_function_with_args(mem, &module, decl, 1024);
    REQUIRE(func != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, decl->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    struct kefir_abi_amd64_type_layout type_layout;
    REQUIRE_OK(kefir_abi_amd64_type_layout(mem, KEFIR_ABI_AMD64_VARIANT_SYSTEM_V,
                                           KEFIR_ABI_AMD64_TYPE_LAYOUT_CONTEXT_GENERIC, func_locals, &type_layout));

    ASSIGN_DECL_CAST(const struct kefir_abi_amd64_typeentry_layout *, entry_layout,
                     kefir_vector_at(&type_layout.layout, 1));

    kefir_irbuilder_block_appendu32_4(mem, &func->body, KEFIR_IR_OPCODE_GET_LOCAL, locals_id, 0, locals_id,
                                      0);                                               // [V, R*]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_PICK, 0);  // [V, R*, R*]
    kefir_irbuilder_block_appendu64(mem, &func->body, KEFIR_IR_OPCODE_UINT_CONST, entry_layout->relative_offset);
    kefir_irbuilder_block_appendu64(mem, &func->body, KEFIR_IR_OPCODE_INT64_ADD,
                                    0);                                                         // [V, R*, R1*]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_PICK, 2);          // [V, R*, R1*, V]
    kefir_irbuilder_block_appendf64(mem, &func->body, KEFIR_IR_OPCODE_FLOAT64_CONST, 3.14159);  // [V, R*, R1*, V, PI]
    kefir_irbuilder_block_appendf64(mem, &func->body, KEFIR_IR_OPCODE_FLOAT64_CONST, 2.0);  // [V, R*, R1*, V, PI, 2f]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_FLOAT64_MUL, 0);      // [V, R*, R1*, V, 2*PI]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_FLOAT64_MUL, 0);      // [V, R*, R1*, 2*PI*V]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT64_STORE, 0);      // [V, R*]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_PICK, 0);      // [V, R*, R*]
    entry_layout = kefir_vector_at(&type_layout.layout, 2);
    kefir_irbuilder_block_appendu64(mem, &func->body, KEFIR_IR_OPCODE_UINT_CONST, entry_layout->relative_offset);
    kefir_irbuilder_block_appendu64(mem, &func->body, KEFIR_IR_OPCODE_INT64_ADD,
                                    0);                                                         // [V, R*, R2*]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_PICK, 2);          // [V, R*, R2*, V]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_PICK, 0);          // [V, R*, R2*, V, V]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_FLOAT64_MUL, 0);          // [V, R*, R2*, V*V]
    kefir_irbuilder_block_appendf64(mem, &func->body, KEFIR_IR_OPCODE_FLOAT64_CONST, 3.14159);  // [V, R*, R2*, V*V, PI]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_FLOAT64_MUL, 0);          // [V, R*, R2*, V*V*PI]
    kefir_irbuilder_block_appendf64(mem, &func->body, KEFIR_IR_OPCODE_FLOAT64_CONST, 2.0);  // [V, R*, R2*, V*V*PI, 2f]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_FLOAT64_DIV, 0);      // [V, R*, R2*, V*V*PI/2]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT64_STORE, 0);      // [V, R*]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_PICK, 0);      // [V, R*, R*]
    entry_layout = kefir_vector_at(&type_layout.layout, 3);
    kefir_irbuilder_block_appendu64(mem, &func->body, KEFIR_IR_OPCODE_UINT_CONST, entry_layout->relative_offset);
    kefir_irbuilder_block_appendu64(mem, &func->body, KEFIR_IR_OPCODE_INT64_ADD,
                                    0);                                                     // [V, R*, R2*]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_PICK, 2);      // [V, R*, R2*, V]
    kefir_irbuilder_block_appendf64(mem, &func->body, KEFIR_IR_OPCODE_FLOAT64_CONST, 0.0);  // [V, R*, R2*, V, 0f]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_EXCHANGE, 1);  // [V, R*, R2*, 0f, V]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_FLOAT64_SUB, 0);      // [V, R*, R2*, -V]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_INT64_STORE, 0);      // [V, R*]
    kefir_irbuilder_block_appendi64(mem, &func->body, KEFIR_IR_OPCODE_RETURN, 0);           // [V, R*]

    KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module);

    REQUIRE_OK(kefir_abi_amd64_type_layout_free(mem, &type_layout));
    KEFIR_CODEGEN_CLOSE(mem, &codegen.iface);
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

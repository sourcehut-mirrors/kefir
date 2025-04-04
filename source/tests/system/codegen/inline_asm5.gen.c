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

#ifdef __x86_64__

    kefir_id_t func_params, func_returns;
    struct kefir_ir_type *decl_params = kefir_ir_module_new_type(mem, &module, 4, &func_params),
                         *decl_result = kefir_ir_module_new_type(mem, &module, 4, &func_returns);
    REQUIRE(decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *decl =
        kefir_ir_module_new_function_declaration(mem, &module, "process", func_params, false, func_returns);
    REQUIRE(decl != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl_params, KEFIR_IR_TYPE_STRUCT, 0, 2));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl_params, KEFIR_IR_TYPE_LONG, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl_params, KEFIR_IR_TYPE_LONG, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl_params, KEFIR_IR_TYPE_LONG, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl_result, KEFIR_IR_TYPE_STRUCT, 0, 2));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl_result, KEFIR_IR_TYPE_LONG, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append(mem, decl_result, KEFIR_IR_TYPE_LONG, 0, 0));
    struct kefir_ir_function *func = kefir_ir_module_new_function_with_args(mem, &module, decl, 1024);
    REQUIRE(func != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, func->name, KEFIR_IR_IDENTIFIER_GLOBAL_DATA));

    kefir_id_t id1;
    struct kefir_ir_inline_assembly *inline_asm1 = kefir_ir_module_new_inline_assembly(mem, &module,
                                                                                       "lea %rax, %1\n"
                                                                                       "add [%rax], %2\n"
                                                                                       "add %2, 1\n"
                                                                                       "add [%rax + 8], %2",
                                                                                       &id1);

    REQUIRE(inline_asm1 != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(kefir_ir_inline_assembly_add_parameter(mem, &module.symbols, inline_asm1, "1",
                                                      KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_LOAD_STORE,
                                                      &(struct kefir_ir_inline_assembly_parameter_constraints) {
                                                          .general_purpose_register = true, .memory_location = true},
                                                      decl_params, func_params, 0, 0, NULL));
    REQUIRE_OK(kefir_ir_inline_assembly_add_parameter(
        mem, &module.symbols, inline_asm1, "2", KEFIR_IR_INLINE_ASSEMBLY_PARAMETER_READ,
        &(struct kefir_ir_inline_assembly_parameter_constraints) {.general_purpose_register = true}, decl_params,
        func_params, 3, 1, NULL));
    REQUIRE_OK(kefir_ir_inline_assembly_add_clobber(mem, &module.symbols, inline_asm1, "rax"));
    REQUIRE_OK(kefir_ir_inline_assembly_add_clobber(mem, &module.symbols, inline_asm1, "cc"));

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &func->body, KEFIR_IR_OPCODE_VSTACK_PICK, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &func->body, KEFIR_IR_OPCODE_INLINE_ASSEMBLY, id1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &func->body, KEFIR_IR_OPCODE_RETURN, 0));
#endif

    KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module);

    KEFIR_CODEGEN_CLOSE(mem, &codegen.iface);
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

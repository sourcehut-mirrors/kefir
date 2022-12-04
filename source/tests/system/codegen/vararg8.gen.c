/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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
#include "kefir/codegen/system-v-amd64.h"

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_codegen_amd64 codegen;
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
    struct kefir_ir_function *sumldouble = kefir_ir_module_new_function(mem, &module, sumldouble_decl, locals_id, 1024);
    REQUIRE(sumldouble != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE_OK(kefir_ir_module_declare_global(mem, &module, sumldouble_decl->name, KEFIR_IR_IDENTIFIER_GLOBAL));

    REQUIRE_OK(kefir_codegen_sysv_amd64_init(mem, &codegen, stdout, NULL));
    codegen.xasmgen.settings.enable_comments = false;

    REQUIRE_OK(kefir_irbuilder_type_append_v(mem, ldouble_type, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append_v(mem, sumldouble_decl_params, KEFIR_IR_TYPE_INT, 0, 0));
    REQUIRE_OK(kefir_irbuilder_type_append_v(mem, sumldouble_decl_result, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));
    REQUIRE_OK(
        kefir_irbuilder_type_append_v(mem, sumldouble_locals, KEFIR_IR_TYPE_BUILTIN, 0, KEFIR_IR_TYPE_BUILTIN_VARARG));
    REQUIRE_OK(kefir_irbuilder_type_append_v(mem, sumldouble_locals, KEFIR_IR_TYPE_LONG_DOUBLE, 0, 0));

    kefir_irbuilder_block_appendu32(mem, &sumldouble->body, KEFIR_IROPCODE_GETLOCAL, locals_id, 0);  // 0: [C, V*]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IROPCODE_VARARG_START, 0);         // 1: [C]
    kefir_irbuilder_block_appendu32(mem, &sumldouble->body, KEFIR_IROPCODE_GETLOCAL, locals_id, 1);  // 2: [C, LD*]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IROPCODE_SETLDH,
                                    kefir_ir_long_double_upper_half(1.0l));          // 3: [C, LD*]
    kefir_irbuilder_block_appendu64(mem, &sumldouble->body, KEFIR_IROPCODE_NOP, 0);  // 4: [C, LD*]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IROPCODE_SETLDL,
                                    kefir_ir_long_double_lower_half(1.0l));                          // 5: [C, LD*]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IROPCODE_POP, 0);                  // 6: [C]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IROPCODE_PICK, 0);                 // 7: [C, C]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IROPCODE_PUSHI64, 0);              // 8: [C, C, 0]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IROPCODE_IEQUALS, 0);              // 9: [C, C==0]
    kefir_irbuilder_block_appendu64(mem, &sumldouble->body, KEFIR_IROPCODE_BRANCH, 19);              // 10: [C] -> @19
    kefir_irbuilder_block_appendu32(mem, &sumldouble->body, KEFIR_IROPCODE_GETLOCAL, locals_id, 0);  // 11: [C, V*]
    kefir_irbuilder_block_appendu32(mem, &sumldouble->body, KEFIR_IROPCODE_VARARG_GET, ldouble_type_id,
                                    0);  // 12: [C, ARG*]
    kefir_irbuilder_block_appendu32(mem, &sumldouble->body, KEFIR_IROPCODE_GETLOCAL, locals_id,
                                    1);  // 13: [C, ARG*, LD*]
    kefir_irbuilder_block_appendu32(mem, &sumldouble->body, KEFIR_IROPCODE_GETLOCAL, locals_id,
                                    1);                                                 // 14: [C, ARG*, LD*, LD*]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IROPCODE_LDADD, 0);   // 15: [C, LD*]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IROPCODE_POP, 0);     // 16: [C]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IROPCODE_IADD1, -1);  // 17: [C-1]
    kefir_irbuilder_block_appendu64(mem, &sumldouble->body, KEFIR_IROPCODE_JMP, 7);     // 18: [C-1] -> @7
    kefir_irbuilder_block_appendu32(mem, &sumldouble->body, KEFIR_IROPCODE_GETLOCAL, locals_id,
                                    0);                                                              // 19: [0, V*]
    kefir_irbuilder_block_appendi64(mem, &sumldouble->body, KEFIR_IROPCODE_VARARG_END, 0);           // 20: [0]
    kefir_irbuilder_block_appendu32(mem, &sumldouble->body, KEFIR_IROPCODE_GETLOCAL, locals_id, 1);  // 21: [0, LD*]

    REQUIRE_OK(KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module));
    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, &codegen.iface));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return EXIT_SUCCESS;
}

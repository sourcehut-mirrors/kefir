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
#include <string.h>
#include "kefir/ir/function.h"
#include "kefir/ir/module.h"
#include "kefir/ir/builder.h"
#include "kefir/core/mem.h"
#include "kefir/core/util.h"
#include "kefir/test/codegen.h"

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_test_codegen codegen;
    kefir_test_codegen_init(mem, &codegen, stdout, NULL);

    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    kefir_id_t func_params, func_returns;
    struct kefir_ir_type *extractu_decl_params = kefir_ir_module_new_type(mem, &module, 0, &func_params),
                         *extractu_decl_result = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(extractu_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(extractu_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *extractu_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "extractu", func_params, false, func_returns);
    REQUIRE(extractu_decl != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function *extractu_func =
        kefir_ir_module_new_function(mem, &module, extractu_decl, KEFIR_ID_NONE, 1024);
    REQUIRE(extractu_func != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 0, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 0, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 0, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 0, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 0, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 0, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 0, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 0, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 0, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 0, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 0, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 1, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 2, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 4, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 8, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 16, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 32, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 1, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 2, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 4, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 8, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extractu_func->body, KEFIR_IROPCODE_EXTUBITS, 16, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extractu_func->body, KEFIR_IROPCODE_STORE64, 0));

    struct kefir_ir_type *extracts_decl_params = kefir_ir_module_new_type(mem, &module, 0, &func_params),
                         *extracts_decl_result = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(extracts_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(extracts_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *extracts_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "extracts", func_params, false, func_returns);
    REQUIRE(extracts_decl != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function *extracts_func =
        kefir_ir_module_new_function(mem, &module, extracts_decl, KEFIR_ID_NONE, 1024);
    REQUIRE(extracts_func != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 0, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 0, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 0, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 0, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 0, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 0, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 0, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 0, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 0, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 0, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 0, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 1, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 2, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 4, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 8, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 16, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 32, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 1, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 2, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 4, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 8, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &extracts_func->body, KEFIR_IROPCODE_EXTSBITS, 16, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &extracts_func->body, KEFIR_IROPCODE_STORE64, 0));

    struct kefir_ir_type *insert_decl_params = kefir_ir_module_new_type(mem, &module, 0, &func_params),
                         *insert_decl_result = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(insert_decl_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(insert_decl_result != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function_decl *insert_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "insert", func_params, false, func_returns);
    REQUIRE(insert_decl != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function *insert_func =
        kefir_ir_module_new_function(mem, &module, insert_decl, KEFIR_ID_NONE, 1024);
    REQUIRE(insert_func != NULL, KEFIR_INTERNAL_ERROR);

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 0, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 0, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 0, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 0, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 0, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 0, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 0, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 0, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 0, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 0, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 0, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));

    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 1, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 2, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 4, 2));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 8, 3));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 16, 4));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 32, 5));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 1, 6));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 2, 7));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 4, 8));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 8, 9));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu32(mem, &insert_func->body, KEFIR_IROPCODE_INSERTBITS, 16, 10));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_PUSHI64, 0));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_XCHG, 1));
    REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &insert_func->body, KEFIR_IROPCODE_STORE64, 0));

    REQUIRE_OK(KEFIR_CODEGEN_TRANSLATE(mem, &codegen.iface, &module));
    REQUIRE_OK(KEFIR_CODEGEN_CLOSE(mem, &codegen.iface));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    return KEFIR_OK;
}

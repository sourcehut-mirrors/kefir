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

#include "kefir/test/module_shim.h"
#include "kefir/ir/builder.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

kefir_result_t kefir_ir_module_declare_global(struct kefir_mem *mem, struct kefir_ir_module *module, const char *symbol,
                                              kefir_ir_identifier_type_t type) {
    struct kefir_ir_identifier identifier = {.symbol = symbol,
                                             .type = type,
                                             .scope = KEFIR_IR_IDENTIFIER_SCOPE_EXPORT,
                                             .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                                             .alias = NULL};

    return kefir_ir_module_declare_identifier(mem, module, symbol, &identifier);
}

kefir_result_t kefir_ir_module_declare_external(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                const char *symbol, kefir_ir_identifier_type_t type) {
    struct kefir_ir_identifier identifier = {.symbol = symbol,
                                             .type = type,
                                             .scope = KEFIR_IR_IDENTIFIER_SCOPE_IMPORT,
                                             .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                                             .alias = NULL};

    return kefir_ir_module_declare_identifier(mem, module, symbol, &identifier);
}

kefir_result_t kefir_ir_module_declare_local(struct kefir_mem *mem, struct kefir_ir_module *module, const char *symbol,
                                             kefir_ir_identifier_type_t type) {
    struct kefir_ir_identifier identifier = {.symbol = symbol,
                                             .type = type,
                                             .scope = KEFIR_IR_IDENTIFIER_SCOPE_LOCAL,
                                             .visibility = KEFIR_IR_IDENTIFIER_VISIBILITY_DEFAULT,
                                             .alias = NULL};

    return kefir_ir_module_declare_identifier(mem, module, symbol, &identifier);
}

kefir_result_t kefir_ir_function_push_arguments(struct kefir_mem *mem, struct kefir_ir_function *function) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(function != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid IR function"));

    for (kefir_size_t i = 0; i < kefir_ir_type_children(function->declaration->params); i++) {
        REQUIRE_OK(kefir_irbuilder_block_appendu64(mem, &function->body, KEFIR_IR_OPCODE_GET_ARGUMENT, i));
    }
    return KEFIR_OK;
}

struct kefir_ir_function *kefir_ir_module_new_function_with_args(struct kefir_mem *mem, struct kefir_ir_module *module,
                                                                 struct kefir_ir_function_decl *decl,
                                                                 kefir_size_t length) {
    struct kefir_ir_function *function = kefir_ir_module_new_function(mem, module, decl, length);
    REQUIRE(function != NULL, NULL);
    kefir_result_t res = kefir_ir_function_push_arguments(mem, function);
    REQUIRE(res == KEFIR_OK, NULL);
    return function;
}

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

#include "kefir/core/mem.h"
#include "kefir/ast-translator/translator.h"
#include "kefir/ast-translator/scope/translator.h"
#include "kefir/ast/local_context.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast-translator/context.h"
#include "kefir/ast-translator/scope/local_scope_layout.h"
#include "kefir/test/util.h"
#include "kefir/ir/builder.h"
#include "kefir/ir/format.h"
#include <stdio.h>

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_ast_translator_environment env;
    REQUIRE_OK(kefir_ast_translator_environment_init(&env, kft_util_get_ir_target_platform()));

    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;
    REQUIRE_OK(
        kefir_ast_global_context_init(mem, kefir_util_default_type_traits(), &env.target_env, &global_context, NULL));
    REQUIRE_OK(kefir_ast_local_context_init(mem, &global_context, &local_context));
    const struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_struct_type *struct_type1 = NULL;
    const struct kefir_ast_type *type1 = kefir_ast_type_structure(mem, context->type_bundle, "", &struct_type1);
    REQUIRE_OK(
        kefir_ast_struct_type_field(mem, context->symbols, struct_type1, "value", kefir_ast_type_signed_int(), NULL));
    REQUIRE_OK(kefir_ast_struct_type_field(mem, context->symbols, struct_type1, "self",
                                           kefir_ast_type_pointer(mem, context->type_bundle, type1), NULL));

    REQUIRE_OK(
        kefir_ast_global_context_declare_external(mem, &global_context, "structure", type1, NULL, NULL, NULL, NULL));

    struct kefir_ast_node_base *node1 = KEFIR_AST_NODE_BASE(kefir_ast_new_struct_indirect_member(
        mem, context->symbols,
        KEFIR_AST_NODE_BASE(kefir_ast_new_struct_member(
            mem, context->symbols, KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context->symbols, "structure")),
            "self")),
        "value"));
    REQUIRE_OK(kefir_ast_analyze_node(mem, context, node1));

    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    struct kefir_ast_translator_global_scope_layout translator_global_scope;
    struct kefir_ast_translator_local_scope_layout translator_local_scope;
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_init(mem, &module, &translator_global_scope));
    REQUIRE_OK(
        kefir_ast_translator_local_scope_layout_init(mem, &module, &translator_global_scope, &translator_local_scope));

    struct kefir_ast_translator_context translator_context;
    REQUIRE_OK(kefir_ast_translator_context_init(mem, &translator_context, context, &env, &module, NULL));
    REQUIRE_OK(
        kefir_ast_translator_build_global_scope_layout(mem, &module, &global_context, &env, &translator_global_scope));
    REQUIRE_OK(
        kefir_ast_translator_build_local_scope_layout(mem, &local_context, &env, &module, &translator_local_scope));
    REQUIRE_OK(kefir_ast_translate_global_scope(mem, &global_context.context, &module, &translator_global_scope));

    kefir_id_t func_params, func_returns;
    struct kefir_ir_type *func1_params = kefir_ir_module_new_type(mem, &module, 0, &func_params);
    struct kefir_ir_type *func1_returns = kefir_ir_module_new_type(mem, &module, 0, &func_returns);
    REQUIRE(func1_params != NULL, KEFIR_INTERNAL_ERROR);
    REQUIRE(func1_returns != NULL, KEFIR_INTERNAL_ERROR);

    struct kefir_ir_function_decl *func1_decl =
        kefir_ir_module_new_function_declaration(mem, &module, "func1", func_params, false, func_returns);
    REQUIRE(func1_decl != NULL, KEFIR_INTERNAL_ERROR);
    struct kefir_ir_function *func1 =
        kefir_ir_module_new_function(mem, &module, func1_decl, translator_local_scope.local_layout_id, 0);
    struct kefir_irbuilder_block builder;
    REQUIRE_OK(kefir_irbuilder_block_init(mem, &builder, &func1->body));
    REQUIRE_OK(kefir_ast_translate_expression(mem, node1, &builder, &translator_context));
    REQUIRE_OK(KEFIR_IRBUILDER_BLOCK_FREE(&builder));
    REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node1));

    REQUIRE_OK(kefir_ir_format_module(stdout, &module));

    REQUIRE_OK(kefir_ast_translator_context_free(mem, &translator_context));
    REQUIRE_OK(kefir_ast_translator_local_scope_layout_free(mem, &translator_local_scope));
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_free(mem, &translator_global_scope));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    REQUIRE_OK(kefir_ast_local_context_free(mem, &local_context));
    REQUIRE_OK(kefir_ast_global_context_free(mem, &global_context));
    return KEFIR_OK;
}

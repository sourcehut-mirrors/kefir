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
#include "./expression.h"
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

    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    struct kefir_ast_statement_expression *expr1 = kefir_ast_new_statement_expression(mem);
    REQUIRE_OK(kefir_ast_analyze_node(mem, context, KEFIR_AST_NODE_BASE(expr1)));

    struct kefir_ast_statement_expression *expr2 = kefir_ast_new_statement_expression(mem);
    REQUIRE_OK(kefir_list_insert_after(mem, &expr2->block_items, kefir_list_tail(&expr2->block_items),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                           mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(mem, 'A'))))));
    REQUIRE_OK(kefir_ast_analyze_node(mem, context, KEFIR_AST_NODE_BASE(expr2)));

    struct kefir_ast_statement_expression *expr3 = kefir_ast_new_statement_expression(mem);
    REQUIRE_OK(kefir_list_insert_after(mem, &expr3->block_items, kefir_list_tail(&expr3->block_items),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                           mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(mem, 'B'))))));
    REQUIRE_OK(kefir_list_insert_after(mem, &expr3->block_items, kefir_list_tail(&expr3->block_items),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                           mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(mem, 'C'))))));
    REQUIRE_OK(kefir_list_insert_after(mem, &expr3->block_items, kefir_list_tail(&expr3->block_items),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                           mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(mem, 'D'))))));
    REQUIRE_OK(kefir_ast_analyze_node(mem, context, KEFIR_AST_NODE_BASE(expr3)));

    struct kefir_ast_statement_expression *expr4 = kefir_ast_new_statement_expression(mem);
    REQUIRE_OK(kefir_list_insert_after(mem, &expr4->block_items, kefir_list_tail(&expr4->block_items),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                           mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(mem, 'E'))))));
    REQUIRE_OK(kefir_list_insert_after(mem, &expr4->block_items, kefir_list_tail(&expr4->block_items),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                           mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(mem, 'F'))))));
    REQUIRE_OK(kefir_list_insert_after(mem, &expr4->block_items, kefir_list_tail(&expr4->block_items),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                           mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(mem, 'G'))))));
    expr4->result = KEFIR_AST_NODE_BASE(
        kefir_ast_new_expression_statement(mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(mem, 'H'))));
    REQUIRE_OK(kefir_ast_analyze_node(mem, context, KEFIR_AST_NODE_BASE(expr4)));

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
    struct kefir_irbuilder_block builder;

    FUNC("statement_expr1", {
        struct kefir_ast_node_base *node = KEFIR_AST_NODE_BASE(expr1);
        REQUIRE_OK(kefir_ast_translate_expression(mem, node, &builder, &translator_context));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    });

    FUNC("statement_expr2", {
        struct kefir_ast_node_base *node = KEFIR_AST_NODE_BASE(expr2);
        REQUIRE_OK(kefir_ast_translate_expression(mem, node, &builder, &translator_context));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    });

    FUNC("statement_expr3", {
        struct kefir_ast_node_base *node = KEFIR_AST_NODE_BASE(expr3);
        REQUIRE_OK(kefir_ast_translate_expression(mem, node, &builder, &translator_context));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    });

    FUNC("statement_expr4", {
        struct kefir_ast_node_base *node = KEFIR_AST_NODE_BASE(expr4);
        REQUIRE_OK(kefir_ast_translate_expression(mem, node, &builder, &translator_context));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    });

    REQUIRE_OK(kefir_ir_format_module(stdout, &module));

    REQUIRE_OK(kefir_ast_translator_context_free(mem, &translator_context));
    REQUIRE_OK(kefir_ast_translator_local_scope_layout_free(mem, &translator_local_scope));
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_free(mem, &translator_global_scope));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    REQUIRE_OK(kefir_ast_local_context_free(mem, &local_context));
    REQUIRE_OK(kefir_ast_global_context_free(mem, &global_context));
    return KEFIR_OK;
}

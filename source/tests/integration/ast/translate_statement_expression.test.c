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
#include "./expression.h"
#include <stdio.h>

kefir_result_t kefir_int_test(struct kefir_mem *mem) {
    struct kefir_ast_translator_environment env;
    REQUIRE_OK(kefir_ast_translator_environment_init(&env, kft_util_get_ir_target_platform()));

    struct kefir_ast_global_context global_context;
    REQUIRE_OK(
        kefir_ast_global_context_init(mem, kefir_util_default_type_traits(), &env.target_env, &global_context, NULL));
    REQUIRE_OK(kefir_ast_global_context_declare_external(mem, &global_context, "variable", kefir_ast_type_signed_int(),
                                                         NULL, NULL, NULL));

    struct kefir_ir_module module;
    REQUIRE_OK(kefir_ir_module_alloc(mem, &module));

    struct kefir_ast_translator_global_scope_layout translator_global_scope;
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_init(mem, &module, &translator_global_scope));

    struct kefir_ast_translator_context global_translator_context;
    REQUIRE_OK(kefir_ast_translator_context_init(mem, &global_translator_context, &global_context.context, &env,
                                                 &module, NULL));
    REQUIRE_OK(
        kefir_ast_translator_build_global_scope_layout(mem, &module, &global_context, &env, &translator_global_scope));
    REQUIRE_OK(kefir_ast_translate_global_scope(mem, &global_context.context, &module, &translator_global_scope));
    struct kefir_irbuilder_block builder;

    FUNC2("expr1", {
        struct kefir_ast_expression_statement *stmt1 =
            kefir_ast_new_expression_statement(mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_long(mem, 10)));

        struct kefir_ast_node_base *node = KEFIR_AST_NODE_BASE(stmt1);
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, node));

        REQUIRE_OK(
            kefir_ast_translator_build_local_scope_layout(mem, &local_context, &env, &module, &translator_local_scope));

        REQUIRE_OK(kefir_ast_translate_statement(mem, node, &builder, &local_translator_context));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    });

    FUNC2("expr2", {
        struct kefir_ast_expression_statement *stmt1 = kefir_ast_new_expression_statement(
            mem, KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
                     mem, KEFIR_AST_OPERATION_ADD, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(mem, 1)),
                     KEFIR_AST_NODE_BASE(kefir_ast_new_constant_float(mem, 4.17f)))));

        struct kefir_ast_node_base *node = KEFIR_AST_NODE_BASE(stmt1);
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, node));

        REQUIRE_OK(
            kefir_ast_translator_build_local_scope_layout(mem, &local_context, &env, &module, &translator_local_scope));

        REQUIRE_OK(kefir_ast_translate_statement(mem, node, &builder, &local_translator_context));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    });

    FUNC2("expr3", {
        struct kefir_ast_expression_statement *stmt1 = kefir_ast_new_expression_statement(
            mem, KEFIR_AST_NODE_BASE(kefir_ast_new_simple_assignment(
                     mem, KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(mem, context->symbols, "variable")),
                     KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(mem, 2)))));

        struct kefir_ast_node_base *node = KEFIR_AST_NODE_BASE(stmt1);
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, node));

        REQUIRE_OK(
            kefir_ast_translator_build_local_scope_layout(mem, &local_context, &env, &module, &translator_local_scope));

        REQUIRE_OK(kefir_ast_translate_statement(mem, node, &builder, &local_translator_context));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    });

    FUNC2("expr4", {
        struct kefir_ast_type_name *type_name1 =
            kefir_ast_new_type_name(mem, kefir_ast_declarator_identifier(mem, NULL, NULL));
        struct kefir_ast_structure_specifier *specifier1 = kefir_ast_structure_specifier_init(mem, NULL, NULL, true);
        struct kefir_ast_structure_declaration_entry *entry1 = kefir_ast_structure_declaration_entry_alloc(mem);
        REQUIRE_OK(kefir_ast_declarator_specifier_list_append(mem, &entry1->declaration.specifiers,
                                                              kefir_ast_type_specifier_char(mem)));
        REQUIRE_OK(kefir_ast_structure_declaration_entry_append(
            mem, entry1, kefir_ast_declarator_identifier(mem, context->symbols, "i"), NULL));
        REQUIRE_OK(kefir_ast_structure_specifier_append_entry(mem, specifier1, entry1));
        REQUIRE_OK(kefir_ast_declarator_specifier_list_append(mem, &type_name1->type_decl.specifiers,
                                                              kefir_ast_type_specifier_struct(mem, specifier1)));

        struct kefir_ast_compound_literal *compound1 = kefir_ast_new_compound_literal(mem, type_name1);
        REQUIRE_OK(kefir_ast_initializer_list_append(
            mem, &compound1->initializer->list, NULL,
            kefir_ast_new_expression_initializer(mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(mem, 100)))));

        struct kefir_ast_type_name *type_name2 =
            kefir_ast_new_type_name(mem, kefir_ast_declarator_identifier(mem, NULL, NULL));
        REQUIRE_OK(kefir_ast_declarator_specifier_list_append(mem, &type_name2->type_decl.specifiers,
                                                              kefir_ast_type_specifier_void(mem)));
        struct kefir_ast_cast_operator *cast1 =
            kefir_ast_new_cast_operator(mem, type_name2, KEFIR_AST_NODE_BASE(compound1));

        struct kefir_ast_expression_statement *stmt1 =
            kefir_ast_new_expression_statement(mem, KEFIR_AST_NODE_BASE(cast1));

        struct kefir_ast_node_base *node = KEFIR_AST_NODE_BASE(stmt1);
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, node));

        REQUIRE_OK(
            kefir_ast_translator_build_local_scope_layout(mem, &local_context, &env, &module, &translator_local_scope));

        REQUIRE_OK(kefir_ast_translate_statement(mem, node, &builder, &local_translator_context));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    });

    FUNC2("expr5", {
        struct kefir_ast_expression_statement *stmt1 = kefir_ast_new_expression_statement(mem, NULL);

        struct kefir_ast_node_base *node = KEFIR_AST_NODE_BASE(stmt1);
        REQUIRE_OK(kefir_ast_analyze_node(mem, context, node));

        REQUIRE_OK(
            kefir_ast_translator_build_local_scope_layout(mem, &local_context, &env, &module, &translator_local_scope));

        REQUIRE_OK(kefir_ast_translate_statement(mem, node, &builder, &local_translator_context));
        REQUIRE_OK(KEFIR_AST_NODE_FREE(mem, node));
    });

    REQUIRE_OK(kefir_ir_format_module(stdout, &module));

    REQUIRE_OK(kefir_ast_translator_context_free(mem, &global_translator_context));
    REQUIRE_OK(kefir_ast_translator_global_scope_layout_free(mem, &translator_global_scope));
    REQUIRE_OK(kefir_ir_module_free(mem, &module));
    REQUIRE_OK(kefir_ast_global_context_free(mem, &global_context));
    return KEFIR_OK;
}

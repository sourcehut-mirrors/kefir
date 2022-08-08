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

#include <string.h>
#include "kefir/test/unit_test.h"
#include "kefir/ast/runtime.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/local_context.h"
#include "kefir/ast/function_declaration_context.h"
#include "kefir/ast/type_conv.h"
#include "kefir/test/util.h"

DEFINE_CASE(ast_node_analysis_static_assertions1, "AST node analysis - static assertions #1") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_define_constant(&kft_mem, &local_context, "X",
                                                      kefir_ast_constant_expression_integer(&kft_mem, 1),
                                                      context->type_traits->underlying_enumeration_type, NULL, NULL));

#define ASSERT_STATIC_OK(_mem, _context, _cond, _err)                                                         \
    do {                                                                                                      \
        struct kefir_ast_static_assertion *assert1 = kefir_ast_new_static_assertion((_mem), (_cond), (_err)); \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(assert1)));                  \
        ASSERT(assert1->base.properties.category == KEFIR_AST_NODE_CATEGORY_INIT_DECLARATOR);                 \
        ASSERT(assert1->base.properties.declaration_props.static_assertion);                                  \
        ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(assert1)));                               \
    } while (0)

#define ASSERT_STATIC_NOK(_mem, _context, _cond, _err)                                                           \
    do {                                                                                                         \
        struct kefir_ast_static_assertion *assert1 = kefir_ast_new_static_assertion((_mem), (_cond), (_err));    \
        ASSERT(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(assert1)) == KEFIR_STATIC_ASSERT); \
        ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(assert1)));                                  \
    } while (0)

    ASSERT_STATIC_OK(&kft_mem, context, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 1)),
                     KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "ErrorA"));

    ASSERT_STATIC_NOK(&kft_mem, context, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 0)),
                      KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "ErrorB"));

    ASSERT_STATIC_OK(&kft_mem, context, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_bool(&kft_mem, true)),
                     KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "Error1"));

    ASSERT_STATIC_NOK(&kft_mem, context, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_bool(&kft_mem, false)),
                      KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "Error2"));

    ASSERT_STATIC_OK(
        &kft_mem, context,
        KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
            &kft_mem, KEFIR_AST_OPERATION_ADD, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 1)),
            KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 2)))),
        KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "Error3"));

    ASSERT_STATIC_NOK(
        &kft_mem, context,
        KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
            &kft_mem, KEFIR_AST_OPERATION_MULTIPLY, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 1000)),
            KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 0)))),
        KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "Error4"));

    ASSERT_STATIC_OK(&kft_mem, context, KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                     KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "Error5"));

    ASSERT_STATIC_NOK(&kft_mem, context,
                      KEFIR_AST_NODE_BASE(kefir_ast_new_unary_operation(
                          &kft_mem, KEFIR_AST_OPERATION_LOGICAL_NEGATE,
                          KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")))),
                      KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "Error6"));

#undef ASSERT_STATIC_OK
#undef ASSERT_STATIC_NOK

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_expression_statements1, "AST node analysis - expression statements #1") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_expression_statement *stmt1 =
        kefir_ast_new_expression_statement(&kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_bool(&kft_mem, true)));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt1)));
    ASSERT(stmt1->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt1->expression->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);

    struct kefir_ast_expression_statement *stmt2 = kefir_ast_new_expression_statement(
        &kft_mem,
        KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
            &kft_mem, KEFIR_AST_OPERATION_ADD, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '\0')),
            KEFIR_AST_NODE_BASE(kefir_ast_new_constant_double(&kft_mem, 6.00001)))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt2)));
    ASSERT(stmt2->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt2->expression->properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);

    struct kefir_ast_expression_statement *stmt3 = kefir_ast_new_expression_statement(&kft_mem, NULL);
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt3)));
    ASSERT(stmt3->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);

    struct kefir_ast_expression_statement *stmt4 = kefir_ast_new_expression_statement(
        &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                      &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_bool(&kft_mem, false)))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt4)));

    struct kefir_ast_type_name *type_name1 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name1->type_decl.specifiers,
                                                         kefir_ast_type_specifier_unsigned(&kft_mem)));

    struct kefir_ast_expression_statement *stmt5 =
        kefir_ast_new_expression_statement(&kft_mem, KEFIR_AST_NODE_BASE(type_name1));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt5)));

    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt1));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt2));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt3));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt4));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt5));
    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_labeled_statements1, "AST node analysis - labeled statements #1") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_labeled_statement *stmt1 = kefir_ast_new_labeled_statement(
        &kft_mem, context->symbols, "label1",
        KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'A')))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt1)));
    ASSERT(stmt1->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt1->label != NULL);
    ASSERT(strcmp(stmt1->label, "label1") == 0);
    ASSERT(stmt1->statement->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);

    struct kefir_ast_labeled_statement *stmt2 = kefir_ast_new_labeled_statement(
        &kft_mem, context->symbols, "label2",
        KEFIR_AST_NODE_BASE(kefir_ast_new_labeled_statement(
            &kft_mem, context->symbols, "label3",
            KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'B')))))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt2)));
    ASSERT(stmt2->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt2->base.properties.statement_props.target_flow_control_point != NULL);
    ASSERT(stmt2->statement->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt2->statement->properties.statement_props.target_flow_control_point != NULL);

    struct kefir_ast_labeled_statement *stmt3 = kefir_ast_new_labeled_statement(
        &kft_mem, context->symbols, "label4", KEFIR_AST_NODE_BASE(kefir_ast_new_constant_double(&kft_mem, 5.0048)));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt3)));

    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt1));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt2));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt3));
    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_case_statements1, "AST node analysis - case statements #1") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_define_constant(&kft_mem, &local_context, "X",
                                                      kefir_ast_constant_expression_integer(&kft_mem, 1004),
                                                      context->type_traits->underlying_enumeration_type, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "whatever",
                                                       kefir_ast_type_signed_int(), NULL, NULL, NULL, NULL));

    struct kefir_hashtree_node *tree_node = NULL;
    struct kefir_ast_flow_control_structure *switch_statement = NULL;
    REQUIRE_OK(kefir_ast_flow_control_tree_push(&kft_mem, context->flow_control_tree,
                                                KEFIR_AST_FLOW_CONTROL_STRUCTURE_SWITCH, &switch_statement));

    struct kefir_ast_case_statement *stmt1 =
        kefir_ast_new_case_statement(&kft_mem, NULL,
                                     KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                         &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '\n')))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt1)));
    ASSERT(stmt1->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt1->base.properties.statement_props.target_flow_control_point != NULL);
    ASSERT(stmt1->base.properties.statement_props.target_flow_control_point ==
           switch_statement->value.switchStatement.defaultCase);
    ASSERT(stmt1->base.properties.statement_props.flow_control_statement == switch_statement);
    ASSERT(stmt1->statement->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);

    struct kefir_ast_case_statement *stmt2 =
        kefir_ast_new_case_statement(&kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 1)),
                                     KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                         &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '\n')))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt2)));
    ASSERT(stmt2->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt2->base.properties.statement_props.target_flow_control_point != NULL);
    ASSERT(stmt2->base.properties.statement_props.flow_control_statement == switch_statement);
    ASSERT(stmt2->statement->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT_OK(kefir_hashtree_at(&switch_statement->value.switchStatement.cases, (kefir_hashtree_key_t) 1, &tree_node));
    ASSERT((void *) tree_node->value == stmt2->base.properties.statement_props.target_flow_control_point);

    struct kefir_ast_case_statement *stmt3 = kefir_ast_new_case_statement(
        &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_ulong_long(&kft_mem, 0xffffe)),
        KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'B')))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt3)));
    ASSERT(stmt3->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt3->base.properties.statement_props.target_flow_control_point != NULL);
    ASSERT(stmt3->base.properties.statement_props.flow_control_statement == switch_statement);
    ASSERT(stmt3->statement->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT_OK(
        kefir_hashtree_at(&switch_statement->value.switchStatement.cases, (kefir_hashtree_key_t) 0xffffe, &tree_node));
    ASSERT((void *) tree_node->value == stmt3->base.properties.statement_props.target_flow_control_point);

    struct kefir_ast_case_statement *stmt4 = kefir_ast_new_case_statement(
        &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_long_long(&kft_mem, -1)),
        KEFIR_AST_NODE_BASE(kefir_ast_new_case_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, ' ')),
            KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'C')))))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt4)));
    ASSERT(stmt4->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt4->base.properties.statement_props.target_flow_control_point != NULL);
    ASSERT(stmt4->base.properties.statement_props.flow_control_statement == switch_statement);
    ASSERT(stmt4->statement->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt4->statement->properties.statement_props.target_flow_control_point != NULL);
    ASSERT(stmt4->statement->properties.statement_props.flow_control_statement == switch_statement);
    ASSERT_OK(kefir_hashtree_at(&switch_statement->value.switchStatement.cases, (kefir_hashtree_key_t) -1, &tree_node));
    ASSERT((void *) tree_node->value == stmt4->base.properties.statement_props.target_flow_control_point);
    ASSERT_OK(
        kefir_hashtree_at(&switch_statement->value.switchStatement.cases, (kefir_hashtree_key_t) ' ', &tree_node));
    ASSERT((void *) tree_node->value == stmt4->statement->properties.statement_props.target_flow_control_point);

    struct kefir_ast_case_statement *stmt5 = kefir_ast_new_case_statement(
        &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_long_long(&kft_mem, -110)),
        KEFIR_AST_NODE_BASE(kefir_ast_new_case_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'X')),
            KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'D')))))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt5)));
    ASSERT(stmt5->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt5->statement->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt5->statement->properties.statement_props.target_flow_control_point != NULL);
    ASSERT(stmt5->statement->properties.statement_props.flow_control_statement == switch_statement);
    ASSERT_OK(
        kefir_hashtree_at(&switch_statement->value.switchStatement.cases, (kefir_hashtree_key_t) -110, &tree_node));
    ASSERT((void *) tree_node->value == stmt5->base.properties.statement_props.target_flow_control_point);

    struct kefir_ast_case_statement *stmt6 =
        kefir_ast_new_case_statement(&kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_float(&kft_mem, 0.0f)),
                                     KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                         &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'E')))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt6)));

    struct kefir_ast_case_statement *stmt7 = kefir_ast_new_case_statement(
        &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
        KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'F')))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt7)));
    ASSERT(stmt7->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt7->base.properties.statement_props.target_flow_control_point != NULL);
    ASSERT(stmt7->base.properties.statement_props.flow_control_statement == switch_statement);
    ASSERT(stmt7->statement->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT_OK(
        kefir_hashtree_at(&switch_statement->value.switchStatement.cases, (kefir_hashtree_key_t) 1004, &tree_node));
    ASSERT((void *) tree_node->value == stmt7->base.properties.statement_props.target_flow_control_point);

    struct kefir_ast_case_statement *stmt8 = kefir_ast_new_case_statement(
        &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "whatever")),
        KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'G')))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt8)));

    struct kefir_ast_case_statement *stmt9 =
        kefir_ast_new_case_statement(&kft_mem, NULL,
                                     KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                         &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '\0')))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt9)));

    struct kefir_ast_case_statement *stmt10 = kefir_ast_new_case_statement(
        &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
        KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '\t')))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt10)));

    struct kefir_ast_flow_control_structure *switch_statement2 = NULL;
    REQUIRE_OK(kefir_ast_flow_control_tree_push(&kft_mem, context->flow_control_tree,
                                                KEFIR_AST_FLOW_CONTROL_STRUCTURE_SWITCH, &switch_statement2));

    struct kefir_ast_case_statement *stmt11 =
        kefir_ast_new_case_statement(&kft_mem, NULL,
                                     KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                         &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '\0')))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt11)));
    ASSERT(stmt11->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt11->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt11->base.properties.statement_props.target_flow_control_point ==
           switch_statement2->value.switchStatement.defaultCase);
    ASSERT(stmt11->base.properties.statement_props.flow_control_statement == switch_statement2);

    struct kefir_ast_case_statement *stmt12 = kefir_ast_new_case_statement(
        &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
        KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '\t')))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt12)));
    ASSERT(stmt12->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt12->base.properties.statement_props.target_flow_control_point != NULL);
    ASSERT(stmt12->base.properties.statement_props.flow_control_statement == switch_statement2);
    ASSERT(stmt12->statement->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT_OK(
        kefir_hashtree_at(&switch_statement2->value.switchStatement.cases, (kefir_hashtree_key_t) 1004, &tree_node));
    ASSERT((void *) tree_node->value == stmt12->base.properties.statement_props.target_flow_control_point);

    REQUIRE_OK(kefir_ast_flow_control_tree_pop(context->flow_control_tree));

    struct kefir_ast_case_statement *stmt13 =
        kefir_ast_new_case_statement(&kft_mem, NULL,
                                     KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                         &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '\0')))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt13)));

    struct kefir_ast_case_statement *stmt14 = kefir_ast_new_case_statement(
        &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
        KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '\t')))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt14)));

    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt1));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt2));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt3));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt4));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt5));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt6));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt7));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt8));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt9));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt10));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt11));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt12));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt13));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt14));
    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_labeled_statements2, "AST node analysis - labeled statements #2") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_flow_control_tree_push(&kft_mem, context->flow_control_tree,
                                               KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK, NULL));

    const struct kefir_ast_scoped_identifier *scoped_id = NULL;
    ASSERT(context->resolve_label_identifier(context, "label1", &scoped_id) == KEFIR_NOT_FOUND);
    ASSERT(context->resolve_label_identifier(context, "label2", &scoped_id) == KEFIR_NOT_FOUND);
    ASSERT(context->resolve_label_identifier(context, "label3", &scoped_id) == KEFIR_NOT_FOUND);
    ASSERT(context->resolve_label_identifier(context, "label4", &scoped_id) == KEFIR_NOT_FOUND);

    struct kefir_ast_labeled_statement *stmt1 = kefir_ast_new_labeled_statement(
        &kft_mem, context->symbols, "label1",
        KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'a')))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt1)));

    ASSERT_OK(context->resolve_label_identifier(context, "label1", &scoped_id));
    ASSERT(scoped_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_LABEL);
    ASSERT(scoped_id->label.point != NULL);
    ASSERT(stmt1->base.properties.statement_props.target_flow_control_point == scoped_id->label.point);

    struct kefir_ast_labeled_statement *stmt2 = kefir_ast_new_labeled_statement(
        &kft_mem, context->symbols, "label1",
        KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'b')))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt2)));

    struct kefir_ast_labeled_statement *stmt3 = kefir_ast_new_labeled_statement(
        &kft_mem, context->symbols, "label2",
        KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'c')))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt3)));

    ASSERT_OK(context->resolve_label_identifier(context, "label2", &scoped_id));
    ASSERT(scoped_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_LABEL);
    ASSERT(scoped_id->label.point != NULL);
    ASSERT(stmt3->base.properties.statement_props.target_flow_control_point == scoped_id->label.point);

    struct kefir_ast_labeled_statement *stmt4;
    struct kefir_ast_labeled_statement *stmt5;
    struct kefir_ast_labeled_statement *stmt6;
    do {
        ASSERT_OK(kefir_ast_local_context_push_block_scope(&kft_mem, &local_context));
        stmt4 = kefir_ast_new_labeled_statement(
            &kft_mem, context->symbols, "label3",
            KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'd')))));
        ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt4)));

        do {
            ASSERT_OK(kefir_ast_local_context_push_block_scope(&kft_mem, &local_context));
            stmt5 = kefir_ast_new_labeled_statement(
                &kft_mem, context->symbols, "label4",
                KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                    &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'e')))));
            ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt5)));

            stmt6 = kefir_ast_new_labeled_statement(
                &kft_mem, context->symbols, "label2",
                KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                    &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'f')))));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt6)));
            ASSERT_OK(kefir_ast_local_context_pop_block_scope(&kft_mem, &local_context));
        } while (0);

        ASSERT_OK(kefir_ast_local_context_pop_block_scope(&kft_mem, &local_context));
    } while (0);

    ASSERT_OK(context->resolve_label_identifier(context, "label3", &scoped_id));
    ASSERT(scoped_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_LABEL);
    ASSERT(scoped_id->label.point != NULL);
    ASSERT(stmt4->base.properties.statement_props.target_flow_control_point == scoped_id->label.point);

    ASSERT_OK(context->resolve_label_identifier(context, "label4", &scoped_id));
    ASSERT(scoped_id->klass == KEFIR_AST_SCOPE_IDENTIFIER_LABEL);
    ASSERT(scoped_id->label.point != NULL);
    ASSERT(stmt5->base.properties.statement_props.target_flow_control_point == scoped_id->label.point);

    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt1));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt2));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt3));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt4));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt5));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt6));
    ASSERT_OK(kefir_ast_flow_control_tree_pop(context->flow_control_tree));
    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_compound_statements1, "AST node analysis - compound statements #1") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));

    struct kefir_ast_compound_statement *stmt1 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, &global_context.context, KEFIR_AST_NODE_BASE(stmt1)));

    struct kefir_ast_compound_statement *stmt2 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &stmt2->block_items, kefir_list_tail(&stmt2->block_items),
                                      KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                          &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 1))))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, &global_context.context, KEFIR_AST_NODE_BASE(stmt2)));

    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt1));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt2));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_compound_statements2, "AST node analysis - compound statements #2") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_function_declaration_context func_decl_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(
        kefir_ast_function_declaration_context_init(&kft_mem, &global_context.context, false, &func_decl_context));

    struct kefir_ast_compound_statement *stmt1 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, &global_context.context, KEFIR_AST_NODE_BASE(stmt1)));

    struct kefir_ast_compound_statement *stmt2 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &stmt2->block_items, kefir_list_tail(&stmt2->block_items),
                                      KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                          &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 1))))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, &global_context.context, KEFIR_AST_NODE_BASE(stmt2)));

    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt1));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt2));
    ASSERT_OK(kefir_ast_function_declaration_context_free(&kft_mem, &func_decl_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_compound_statements3, "AST node analysis - compound statements #3") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_compound_statement *stmt1 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt1)));
    ASSERT(stmt1->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt1->base.properties.statement_props.flow_control_statement != NULL);
    ASSERT(stmt1->base.properties.statement_props.flow_control_statement->type ==
           KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK);

    struct kefir_ast_declaration *decl1 = kefir_ast_new_single_declaration(
        &kft_mem, kefir_ast_declarator_identifier(&kft_mem, context->symbols, "var1"),
        kefir_ast_new_expression_initializer(&kft_mem,
                                             KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '\t'))),
        NULL);
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &decl1->specifiers,
                                                         kefir_ast_type_qualifier_volatile(&kft_mem)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &decl1->specifiers,
                                                         kefir_ast_type_specifier_unsigned(&kft_mem)));

    struct kefir_ast_declaration *decl2 = kefir_ast_new_single_declaration(
        &kft_mem,
        kefir_ast_declarator_pointer(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, context->symbols, "X")), NULL,
        NULL);
    struct kefir_ast_structure_specifier *specifier1 = kefir_ast_structure_specifier_init(&kft_mem, NULL, NULL, true);
    struct kefir_ast_structure_declaration_entry *entry1 = kefir_ast_structure_declaration_entry_alloc(&kft_mem);
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &entry1->declaration.specifiers,
                                                         kefir_ast_type_specifier_float(&kft_mem)));
    ASSERT_OK(kefir_ast_structure_declaration_entry_append(
        &kft_mem, entry1,
        kefir_ast_declarator_array(
            &kft_mem, KEFIR_AST_DECLARATOR_ARRAY_BOUNDED,
            KEFIR_AST_NODE_BASE(kefir_ast_new_unary_operation(
                &kft_mem, KEFIR_AST_OPERATION_SIZEOF,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "var1")))),
            kefir_ast_declarator_identifier(&kft_mem, context->symbols, "arr")),
        NULL));
    ASSERT_OK(kefir_ast_structure_specifier_append_entry(&kft_mem, specifier1, entry1));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &decl2->specifiers,
                                                         kefir_ast_storage_class_specifier_typedef(&kft_mem)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &decl2->specifiers,
                                                         kefir_ast_type_specifier_struct(&kft_mem, specifier1)));

    struct kefir_ast_compound_statement *stmt2 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &stmt2->block_items, kefir_list_tail(&stmt2->block_items),
                                      KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                          &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '#'))))));
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &stmt2->block_items, kefir_list_tail(&stmt2->block_items),
                                      KEFIR_AST_NODE_BASE(decl1)));
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &stmt2->block_items, kefir_list_tail(&stmt2->block_items),
                                      KEFIR_AST_NODE_BASE(decl2)));
    ASSERT_OK(kefir_list_insert_after(
        &kft_mem, &stmt2->block_items, kefir_list_tail(&stmt2->block_items),
        KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
            &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_simple_assignment(
                          &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "var1")),
                          KEFIR_AST_NODE_BASE(kefir_ast_new_unary_operation(
                              &kft_mem, KEFIR_AST_OPERATION_ALIGNOF,
                              KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X"))))))))));
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &stmt2->block_items, kefir_list_tail(&stmt2->block_items),
                                      KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(&kft_mem, NULL))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt2)));
    ASSERT(stmt2->base.properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);
    ASSERT(stmt2->base.properties.statement_props.flow_control_statement != NULL);
    ASSERT(stmt2->base.properties.statement_props.flow_control_statement->type ==
           KEFIR_AST_FLOW_CONTROL_STRUCTURE_BLOCK);

    const struct kefir_list_entry *iter = kefir_list_head(&stmt2->block_items);
    ASSERT(iter != NULL);
    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, item1, iter->value);
    ASSERT(item1->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);

    kefir_list_next(&iter);
    ASSERT(iter != NULL);
    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, item2, iter->value);
    ASSERT(item2->properties.category == KEFIR_AST_NODE_CATEGORY_DECLARATION);
    ASSIGN_DECL_CAST(struct kefir_ast_declaration *, item2_decl_list, item2->self);
    ASSERT(kefir_list_length(&item2_decl_list->init_declarators) == 1);
    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, item2_decl,
                     kefir_list_head(&item2_decl_list->init_declarators)->value);
    ASSERT(strcmp(item2_decl->properties.declaration_props.identifier, "var1") == 0);
    ASSERT(item2_decl->properties.declaration_props.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_AUTO);
    ASSERT(item2_decl->properties.declaration_props.alignment == 0);
    ASSERT(item2_decl->properties.declaration_props.function == KEFIR_AST_FUNCTION_SPECIFIER_NONE);
    ASSERT(KEFIR_AST_TYPE_SAME(item2_decl->properties.type,
                               kefir_ast_type_qualified(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_int(),
                                                        (struct kefir_ast_type_qualification){.volatile_type = true})));

    kefir_list_next(&iter);
    ASSERT(iter != NULL);
    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, item3, iter->value);
    ASSERT(item3->properties.category == KEFIR_AST_NODE_CATEGORY_DECLARATION);
    ASSIGN_DECL_CAST(struct kefir_ast_declaration *, item3_decl_list, item3->self);
    ASSERT(kefir_list_length(&item3_decl_list->init_declarators) == 1);
    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, item3_decl,
                     kefir_list_head(&item3_decl_list->init_declarators)->value);
    ASSERT(strcmp(item3_decl->properties.declaration_props.identifier, "X") == 0);
    ASSERT(item3_decl->properties.declaration_props.storage == KEFIR_AST_SCOPE_IDENTIFIER_STORAGE_TYPEDEF);
    ASSERT(item3_decl->properties.declaration_props.alignment == 0);
    ASSERT(item3_decl->properties.declaration_props.function == KEFIR_AST_FUNCTION_SPECIFIER_NONE);

    struct kefir_ast_struct_type *struct_type1 = NULL;
    const struct kefir_ast_type *type1 = kefir_ast_type_structure(&kft_mem, context->type_bundle, NULL, &struct_type1);
    ASSERT_OK(
        kefir_ast_struct_type_field(&kft_mem, context->symbols, struct_type1, "arr",
                                    kefir_ast_type_array(&kft_mem, context->type_bundle, kefir_ast_type_float(),
                                                         kefir_ast_constant_expression_integer(&kft_mem, 4), NULL),
                                    NULL));
    ASSERT(KEFIR_AST_TYPE_SAME(item3_decl->properties.type,
                               kefir_ast_type_pointer(&kft_mem, context->type_bundle, type1)));

    kefir_list_next(&iter);
    ASSERT(iter != NULL);
    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, item4, iter->value);
    ASSERT(item4->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);

    kefir_list_next(&iter);
    ASSERT(iter != NULL);
    ASSIGN_DECL_CAST(struct kefir_ast_node_base *, item5, iter->value);
    ASSERT(item5->properties.category == KEFIR_AST_NODE_CATEGORY_STATEMENT);

    kefir_list_next(&iter);
    ASSERT(iter == NULL);

    const struct kefir_ast_scoped_identifier *scoped_id = NULL;
    ASSERT(context->resolve_ordinary_identifier(context, "var1", &scoped_id) == KEFIR_NOT_FOUND);
    ASSERT(context->resolve_ordinary_identifier(context, "X", &scoped_id) == KEFIR_NOT_FOUND);

    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt1));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt2));
    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_compound_statements4, "AST node analysis - compound statements #4") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_compound_statement *stmt1 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &stmt1->block_items, kefir_list_tail(&stmt1->block_items),
                                      KEFIR_AST_NODE_BASE(kefir_ast_new_constant_double(&kft_mem, 4.15))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt1)));

    struct kefir_ast_type_name *type_name1 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name1->type_decl.specifiers,
                                                         kefir_ast_type_specifier_double(&kft_mem)));

    struct kefir_ast_compound_statement *stmt2 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &stmt2->block_items, kefir_list_tail(&stmt2->block_items),
                                      KEFIR_AST_NODE_BASE(type_name1)));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt2)));

    struct kefir_ast_type_name *type_name2 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name2->type_decl.specifiers,
                                                         kefir_ast_type_specifier_int(&kft_mem)));

    struct kefir_ast_compound_statement *stmt3 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &stmt3->block_items, kefir_list_tail(&stmt3->block_items),
                                      KEFIR_AST_NODE_BASE(kefir_ast_new_static_assertion(
                                          &kft_mem,
                                          KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
                                              &kft_mem, KEFIR_AST_OPERATION_EQUAL,
                                              KEFIR_AST_NODE_BASE(kefir_ast_new_unary_operation(
                                                  &kft_mem, KEFIR_AST_OPERATION_SIZEOF,
                                                  KEFIR_AST_NODE_CLONE(&kft_mem, KEFIR_AST_NODE_BASE(type_name2)))),
                                              KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 2)))),
                                          KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "ERROR!")))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt3)));

    struct kefir_ast_compound_statement *stmt4 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_OK(
        kefir_list_insert_after(&kft_mem, &stmt4->block_items, kefir_list_tail(&stmt4->block_items),
                                KEFIR_AST_NODE_BASE(kefir_ast_new_static_assertion(
                                    &kft_mem,
                                    KEFIR_AST_NODE_BASE(kefir_ast_new_binary_operation(
                                        &kft_mem, KEFIR_AST_OPERATION_EQUAL,
                                        KEFIR_AST_NODE_BASE(kefir_ast_new_unary_operation(
                                            &kft_mem, KEFIR_AST_OPERATION_SIZEOF, KEFIR_AST_NODE_BASE(type_name2))),
                                        KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 4)))),
                                    KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "ERROR2!")))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt4)));

    struct kefir_ast_compound_statement *stmt5 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &stmt5->block_items, kefir_list_tail(&stmt5->block_items),
                                      KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(&kft_mem, NULL))));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(stmt5)));

    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt1));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt2));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt3));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt4));
    KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(stmt5));
    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

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
#include "kefir/ast/node.h"

DEFINE_CASE(ast_nodes_goto_statements1, "AST nodes - goto statements #1") {
    struct kefir_symbol_table symbols;
    struct kefir_ast_type_bundle type_bundle;

    ASSERT_OK(kefir_symbol_table_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));

    struct kefir_ast_goto_statement *goto1 = kefir_ast_new_goto_statement(&kft_mem, &symbols, "label1");
    ASSERT(goto1 != NULL);
    ASSERT(goto1->base.klass->type == KEFIR_AST_GOTO_STATEMENT);
    ASSERT(goto1->base.self == goto1);
    ASSERT(goto1->identifier != NULL);
    ASSERT(strcmp(goto1->identifier, "label1") == 0);

    struct kefir_ast_goto_statement *goto2 = kefir_ast_new_goto_statement(&kft_mem, &symbols, "label2");
    ASSERT(goto2 != NULL);
    ASSERT(goto2->base.klass->type == KEFIR_AST_GOTO_STATEMENT);
    ASSERT(goto2->base.self == goto2);
    ASSERT(goto2->identifier != NULL);
    ASSERT(strcmp(goto2->identifier, "label2") == 0);

    struct kefir_ast_goto_statement *goto3 = kefir_ast_new_goto_statement(&kft_mem, NULL, "label3");
    ASSERT(goto3 != NULL);
    ASSERT(goto3->base.klass->type == KEFIR_AST_GOTO_STATEMENT);
    ASSERT(goto3->base.self == goto3);
    ASSERT(goto3->identifier != NULL);
    ASSERT(strcmp(goto3->identifier, "label3") == 0);

    struct kefir_ast_goto_statement *goto4 = kefir_ast_new_goto_statement(&kft_mem, NULL, NULL);
    ASSERT(goto4 == NULL);
    struct kefir_ast_goto_statement *goto5 = kefir_ast_new_goto_statement(&kft_mem, &symbols, NULL);
    ASSERT(goto5 == NULL);

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(goto1)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(goto2)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(goto3)));
    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_symbol_table_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_nodes_goto_address_statements1, "AST nodes - goto address statements #1") {
    struct kefir_symbol_table symbols;
    struct kefir_ast_type_bundle type_bundle;

    ASSERT_OK(kefir_symbol_table_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));

    struct kefir_ast_goto_statement *goto1 =
        kefir_ast_new_goto_address_statement(&kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 0)));
    ASSERT(goto1 != NULL);
    ASSERT(goto1->base.klass->type == KEFIR_AST_GOTO_ADDRESS_STATEMENT);
    ASSERT(goto1->base.self == goto1);
    ASSERT(goto1->target != NULL);
    ASSERT(goto1->target->klass->type == KEFIR_AST_CONSTANT);

    struct kefir_ast_goto_statement *goto2 = kefir_ast_new_goto_address_statement(
        &kft_mem, KEFIR_AST_NODE_BASE(KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "Test..test..test...")));
    ASSERT(goto2 != NULL);
    ASSERT(goto2->base.klass->type == KEFIR_AST_GOTO_ADDRESS_STATEMENT);
    ASSERT(goto2->base.self == goto2);
    ASSERT(goto2->target != NULL);
    ASSERT(goto2->target->klass->type == KEFIR_AST_STRING_LITERAL);

    struct kefir_ast_goto_statement *goto3 = kefir_ast_new_goto_address_statement(&kft_mem, NULL);
    ASSERT(goto3 == NULL);

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(goto1)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(goto2)));
    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_symbol_table_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_nodes_continue_statements, "AST nodes - continue statements") {
    struct kefir_symbol_table symbols;
    struct kefir_ast_type_bundle type_bundle;

    ASSERT_OK(kefir_symbol_table_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));

    struct kefir_ast_continue_statement *continue1 = kefir_ast_new_continue_statement(&kft_mem);
    ASSERT(continue1 != NULL);
    ASSERT(continue1->base.klass->type == KEFIR_AST_CONTINUE_STATEMENT);
    ASSERT(continue1->base.self == continue1);

    struct kefir_ast_continue_statement *continue2 = kefir_ast_new_continue_statement(&kft_mem);
    ASSERT(continue2 != NULL);
    ASSERT(continue2->base.klass->type == KEFIR_AST_CONTINUE_STATEMENT);
    ASSERT(continue2->base.self == continue2);
    ASSERT(continue1 != continue2);

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(continue1)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(continue2)));
    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_symbol_table_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_nodes_break_statements, "AST nodes - break statements") {
    struct kefir_symbol_table symbols;
    struct kefir_ast_type_bundle type_bundle;

    ASSERT_OK(kefir_symbol_table_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));

    struct kefir_ast_break_statement *break1 = kefir_ast_new_break_statement(&kft_mem);
    ASSERT(break1 != NULL);
    ASSERT(break1->base.klass->type == KEFIR_AST_BREAK_STATEMENT);
    ASSERT(break1->base.self == break1);

    struct kefir_ast_break_statement *break2 = kefir_ast_new_break_statement(&kft_mem);
    ASSERT(break2 != NULL);
    ASSERT(break2->base.klass->type == KEFIR_AST_BREAK_STATEMENT);
    ASSERT(break2->base.self == break2);
    ASSERT(break1 != break2);

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(break1)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(break2)));
    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_symbol_table_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_nodes_return_statements1, "AST nodes - return statements #1") {
    struct kefir_symbol_table symbols;
    struct kefir_ast_type_bundle type_bundle;

    ASSERT_OK(kefir_symbol_table_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));

    struct kefir_ast_return_statement *return1 =
        kefir_ast_new_return_statement(&kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 1)));
    ASSERT(return1 != NULL);
    ASSERT(return1->base.klass->type == KEFIR_AST_RETURN_STATEMENT);
    ASSERT(return1->base.self == return1);
    ASSERT(return1->expression != NULL);
    ASSERT(return1->expression->klass->type == KEFIR_AST_CONSTANT);
    ASSERT(((struct kefir_ast_constant *) return1->expression->self)->type == KEFIR_AST_INT_CONSTANT);
    ASSERT(((struct kefir_ast_constant *) return1->expression->self)->value.integer == 1);

    struct kefir_ast_return_statement *return2 = kefir_ast_new_return_statement(&kft_mem, NULL);
    ASSERT(return2 != NULL);
    ASSERT(return2->base.klass->type == KEFIR_AST_RETURN_STATEMENT);
    ASSERT(return2->base.self == return2);
    ASSERT(return2->expression == NULL);

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(return1)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(return2)));
    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_symbol_table_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_nodes_function_definitions1, "AST nodes - function definitions #1") {
    struct kefir_symbol_table symbols;
    struct kefir_ast_type_bundle type_bundle;

    ASSERT_OK(kefir_symbol_table_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));

    struct kefir_ast_declaration *param1 = kefir_ast_new_single_declaration(
        &kft_mem, kefir_ast_declarator_identifier(&kft_mem, &symbols, "x"), NULL, NULL);
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &param1->specifiers,
                                                         kefir_ast_type_specifier_long(&kft_mem)));

    struct kefir_ast_declaration *param2 = kefir_ast_new_single_declaration(
        &kft_mem, kefir_ast_declarator_identifier(&kft_mem, &symbols, "y"), NULL, NULL);
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &param1->specifiers,
                                                         kefir_ast_type_specifier_double(&kft_mem)));

    struct kefir_ast_declarator *decl1 =
        kefir_ast_declarator_function(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, &symbols, "fn1"));
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &decl1->function.parameters,
                                      kefir_list_tail(&decl1->function.parameters), param1));

    struct kefir_ast_compound_statement *body1 = kefir_ast_new_compound_statement(&kft_mem);
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &body1->block_items, kefir_list_tail(&body1->block_items),
                                      KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(&kft_mem, NULL))));

    struct kefir_ast_function_definition *func1 = kefir_ast_new_function_definition(&kft_mem, decl1, body1);
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &func1->specifiers,
                                                         kefir_ast_type_specifier_int(&kft_mem)));
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &func1->declarations, kefir_list_tail(&func1->declarations), param2));

    ASSERT(func1 != NULL);
    ASSERT(func1->base.klass->type == KEFIR_AST_FUNCTION_DEFINITION);
    ASSERT(func1->base.self == func1);

    struct kefir_ast_declarator_specifier *specifier1 = NULL;
    struct kefir_list_entry *iter = kefir_ast_declarator_specifier_list_iter(&func1->specifiers, &specifier1);
    ASSERT(iter != NULL);
    ASSERT(specifier1->klass == KEFIR_AST_TYPE_SPECIFIER);
    ASSERT(specifier1->type_specifier.specifier == KEFIR_AST_TYPE_SPECIFIER_INT);
    ASSERT_OK(kefir_ast_declarator_specifier_list_next(&iter, &specifier1));
    ASSERT(iter == NULL);

    ASSERT(func1->declarator == decl1);
    const struct kefir_list_entry *iter2 = kefir_list_head(&func1->declarations);
    ASSERT(iter2 != NULL);
    ASSERT(iter2->value == param2);
    kefir_list_next(&iter2);
    ASSERT(iter2 == NULL);
    ASSERT(func1->body == body1);

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(func1)));
    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_symbol_table_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_nodes_translation_units1, "AST nodes - translation units #1") {
    struct kefir_symbol_table symbols;
    struct kefir_ast_type_bundle type_bundle;

    ASSERT_OK(kefir_symbol_table_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));

    struct kefir_ast_translation_unit *unit1 = kefir_ast_new_translation_unit(&kft_mem);
    ASSERT(unit1 != NULL);
    ASSERT(unit1->base.klass->type == KEFIR_AST_TRANSLATION_UNIT);
    ASSERT(unit1->base.self == unit1);
    ASSERT(kefir_list_length(&unit1->external_definitions) == 0);

    struct kefir_ast_declaration *decl1 = kefir_ast_new_single_declaration(
        &kft_mem, kefir_ast_declarator_identifier(&kft_mem, &symbols, "xyz"),
        kefir_ast_new_expression_initializer(&kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'a'))),
        NULL);
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &decl1->specifiers,
                                                         kefir_ast_type_specifier_char(&kft_mem)));
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &unit1->external_definitions,
                                      kefir_list_tail(&unit1->external_definitions), KEFIR_AST_NODE_BASE(decl1)));

    struct kefir_ast_declaration *decl2 = kefir_ast_new_single_declaration(
        &kft_mem, kefir_ast_declarator_pointer(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, &symbols, "abc")),
        kefir_ast_new_expression_initializer(&kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 0))),
        NULL);
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &decl2->specifiers,
                                                         kefir_ast_type_specifier_void(&kft_mem)));
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &unit1->external_definitions,
                                      kefir_list_tail(&unit1->external_definitions), KEFIR_AST_NODE_BASE(decl2)));

    struct kefir_ast_declarator *decl3 =
        kefir_ast_declarator_function(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, &symbols, "fn1"));
    struct kefir_ast_compound_statement *body1 = kefir_ast_new_compound_statement(&kft_mem);
    struct kefir_ast_function_definition *func1 = kefir_ast_new_function_definition(&kft_mem, decl3, body1);
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &func1->specifiers,
                                                         kefir_ast_type_specifier_void(&kft_mem)));
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &unit1->external_definitions,
                                      kefir_list_tail(&unit1->external_definitions), KEFIR_AST_NODE_BASE(func1)));

    const struct kefir_list_entry *iter = kefir_list_head(&unit1->external_definitions);
    ASSERT(iter != NULL);
    ASSERT(iter->value == KEFIR_AST_NODE_BASE(decl1));
    kefir_list_next(&iter);
    ASSERT(iter != NULL);
    ASSERT(iter->value == KEFIR_AST_NODE_BASE(decl2));
    kefir_list_next(&iter);
    ASSERT(iter != NULL);
    ASSERT(iter->value == KEFIR_AST_NODE_BASE(func1));
    kefir_list_next(&iter);
    ASSERT(iter == NULL);

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(unit1)));
    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_symbol_table_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_nodes_declaration1, "AST nodes - declaration list #1") {
    struct kefir_symbol_table symbols;
    struct kefir_ast_type_bundle type_bundle;

    ASSERT_OK(kefir_symbol_table_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));

    struct kefir_ast_declaration *declaration = kefir_ast_new_declaration(&kft_mem);
    ASSERT(declaration != NULL);
    ASSERT(declaration->base.klass->type == KEFIR_AST_DECLARATION);
    ASSERT(declaration->base.self == declaration);
    ASSERT(kefir_list_length(&declaration->init_declarators) == 0);
    struct kefir_ast_declarator_specifier *specifier = NULL;
    ASSERT(kefir_ast_declarator_specifier_list_iter(&declaration->specifiers, &specifier) == NULL);

    struct kefir_ast_init_declarator *decl1 =
        kefir_ast_new_init_declarator(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, &symbols, "A"), NULL);
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &declaration->init_declarators,
                                      kefir_list_tail(&declaration->init_declarators), KEFIR_AST_NODE_BASE(decl1)));
    struct kefir_ast_init_declarator *decl2 =
        kefir_ast_new_init_declarator(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, &symbols, "B"), NULL);
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &declaration->init_declarators,
                                      kefir_list_tail(&declaration->init_declarators), KEFIR_AST_NODE_BASE(decl2)));
    struct kefir_ast_init_declarator *decl3 =
        kefir_ast_new_init_declarator(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, &symbols, "B"), NULL);
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &declaration->init_declarators,
                                      kefir_list_tail(&declaration->init_declarators), KEFIR_AST_NODE_BASE(decl3)));

    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &declaration->specifiers,
                                                         kefir_ast_type_specifier_int(&kft_mem)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &declaration->specifiers,
                                                         kefir_ast_type_qualifier_const(&kft_mem)));

    ASSERT(kefir_list_length(&declaration->init_declarators) == 3);
    const struct kefir_list_entry *iter = kefir_list_head(&declaration->init_declarators);
    ASSERT(iter != NULL);
    ASSERT(iter->value == KEFIR_AST_NODE_BASE(decl1));
    kefir_list_next(&iter);
    ASSERT(iter != NULL);
    ASSERT(iter->value == KEFIR_AST_NODE_BASE(decl2));
    kefir_list_next(&iter);
    ASSERT(iter != NULL);
    ASSERT(iter->value == KEFIR_AST_NODE_BASE(decl3));
    kefir_list_next(&iter);
    ASSERT(iter == NULL);

    struct kefir_list_entry *iter2 = kefir_ast_declarator_specifier_list_iter(&declaration->specifiers, &specifier);
    ASSERT(iter2 != NULL && specifier != NULL);
    ASSERT(specifier->klass == KEFIR_AST_TYPE_SPECIFIER);
    ASSERT(specifier->type_specifier.specifier == KEFIR_AST_TYPE_SPECIFIER_INT);
    ASSERT_OK(kefir_ast_declarator_specifier_list_next(&iter2, &specifier));
    ASSERT(iter2 != NULL && specifier != NULL);
    ASSERT(specifier->klass == KEFIR_AST_TYPE_QUALIFIER);
    ASSERT(specifier->type_qualifier == KEFIR_AST_TYPE_QUALIFIER_CONST);
    ASSERT_OK(kefir_ast_declarator_specifier_list_next(&iter2, &specifier));
    ASSERT(iter2 == NULL);

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(declaration)));
    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_symbol_table_free(&kft_mem, &symbols));
}
END_CASE

static kefir_result_t extension_free(struct kefir_mem *mem, struct kefir_ast_extension_node *ext) {
    KEFIR_FREE(mem, ext->payload);
    ext->payload = NULL;
    return KEFIR_OK;
}

static kefir_result_t extension_clone(struct kefir_mem *mem, struct kefir_ast_extension_node *dst,
                                      const struct kefir_ast_extension_node *src) {
    UNUSED(mem);
    UNUSED(dst);
    UNUSED(src);
    return KEFIR_INTERNAL_ERROR;
}

DEFINE_CASE(ast_nodes_extension, "AST nodes - extensions") {
    struct kefir_ast_extension_node_class ext_class = {
        .free = extension_free, .clone = extension_clone, .payload = NULL};

    void *buf1 = KEFIR_MALLOC(&kft_mem, 1024);
    void *buf2 = KEFIR_MALLOC(&kft_mem, 2048);
    struct kefir_ast_extension_node *ext1 = kefir_ast_new_extension_node(&kft_mem, &ext_class, buf1);
    ASSERT(ext1 != NULL);
    ASSERT(ext1->base.klass->type == KEFIR_AST_EXTENSION_NODE);
    ASSERT(ext1->base.self == ext1);
    ASSERT(ext1->klass == &ext_class);
    ASSERT(ext1->payload == buf1);

    struct kefir_ast_extension_node *ext2 = kefir_ast_new_extension_node(&kft_mem, &ext_class, buf2);
    ASSERT(ext2 != NULL);
    ASSERT(ext2->base.klass->type == KEFIR_AST_EXTENSION_NODE);
    ASSERT(ext2->base.self == ext2);
    ASSERT(ext2->klass == &ext_class);
    ASSERT(ext2->payload == buf2);

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(ext1)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(ext2)));
}
END_CASE

DEFINE_CASE(ast_nodes_statement_expressions1, "AST nodes - statement expressions #1") {
    struct kefir_symbol_table symbols;
    struct kefir_ast_type_bundle type_bundle;

    ASSERT_OK(kefir_symbol_table_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));

    struct kefir_ast_statement_expression *expr1 = kefir_ast_new_statement_expression(&kft_mem);
    ASSERT(expr1 != NULL);
    ASSERT(expr1->base.klass->type == KEFIR_AST_STATEMENT_EXPRESSION);
    ASSERT(expr1->base.self == expr1);
    ASSERT(kefir_list_length(&expr1->block_items) == 0);
    ASSERT(expr1->result == NULL);

    ASSERT_OK(kefir_list_insert_after(&kft_mem, &expr1->block_items, kefir_list_tail(&expr1->block_items),
                                      KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                          &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '1'))))));
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &expr1->block_items, kefir_list_tail(&expr1->block_items),
                                      KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                          &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '2'))))));
    ASSERT_OK(kefir_list_insert_after(&kft_mem, &expr1->block_items, kefir_list_tail(&expr1->block_items),
                                      KEFIR_AST_NODE_BASE(kefir_ast_new_expression_statement(
                                          &kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '3'))))));
    ASSERT(kefir_list_length(&expr1->block_items) == 3);
    expr1->result = KEFIR_AST_NODE_BASE(
        kefir_ast_new_expression_statement(&kft_mem, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, '4'))));

    struct kefir_ast_statement_expression *expr2 = kefir_ast_new_statement_expression(&kft_mem);
    ASSERT(expr2 != NULL);
    ASSERT(expr2->base.klass->type == KEFIR_AST_STATEMENT_EXPRESSION);
    ASSERT(expr2->base.self == expr2);
    ASSERT(kefir_list_length(&expr2->block_items) == 0);
    ASSERT(expr2->result == NULL);

    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(expr1)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(expr2)));
    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_symbol_table_free(&kft_mem, &symbols));
}
END_CASE

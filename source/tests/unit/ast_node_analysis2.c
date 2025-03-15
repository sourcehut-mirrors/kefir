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

#include <string.h>
#include "kefir/test/unit_test.h"
#include "kefir/ast/analyzer/analyzer.h"
#include "kefir/ast/local_context.h"
#include "kefir/ast/type_conv.h"
#include "kefir/test/util.h"

#define ASSERT_GENERIC_SELECTION(_mem, _context, _id, _type, _lvalue, _const, _addressable)                            \
    do {                                                                                                               \
        struct kefir_ast_type_name *type_name1 =                                                                       \
            kefir_ast_new_type_name((_mem), kefir_ast_declarator_identifier((_mem), NULL, NULL));                      \
        ASSERT_OK(kefir_ast_declarator_specifier_list_append((_mem), &type_name1->type_decl.specifiers,                \
                                                             kefir_ast_type_specifier_char((_mem))));                  \
        struct kefir_ast_type_name *type_name2 =                                                                       \
            kefir_ast_new_type_name((_mem), kefir_ast_declarator_identifier((_mem), NULL, NULL));                      \
        ASSERT_OK(kefir_ast_declarator_specifier_list_append((_mem), &type_name2->type_decl.specifiers,                \
                                                             kefir_ast_type_specifier_unsigned((_mem))));              \
        struct kefir_ast_type_name *type_name3 =                                                                       \
            kefir_ast_new_type_name((_mem), kefir_ast_declarator_identifier((_mem), NULL, NULL));                      \
        ASSERT_OK(kefir_ast_declarator_specifier_list_append((_mem), &type_name3->type_decl.specifiers,                \
                                                             kefir_ast_type_specifier_double((_mem))));                \
        struct kefir_ast_type_name *type_name4 = kefir_ast_new_type_name(                                              \
            (_mem), kefir_ast_declarator_pointer((_mem), kefir_ast_declarator_identifier((_mem), NULL, NULL)));        \
        ASSERT_OK(kefir_ast_declarator_specifier_list_append((_mem), &type_name4->type_decl.specifiers,                \
                                                             kefir_ast_type_specifier_void((_mem))));                  \
        struct kefir_ast_generic_selection *selection1 = kefir_ast_new_generic_selection(                              \
            (_mem), KEFIR_AST_NODE_BASE(kefir_ast_new_identifier((_mem), (_context)->symbols, (_id))));                \
        ASSERT_OK(kefir_ast_generic_selection_append(                                                                  \
            (_mem), selection1, type_name1,                                                                            \
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier((_mem), (_context)->symbols, "bool"))));                      \
        ASSERT_OK(kefir_ast_generic_selection_append(                                                                  \
            (_mem), selection1, type_name2, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_float((_mem), 3.14f))));        \
        ASSERT_OK(kefir_ast_generic_selection_append((_mem), selection1, type_name3,                                   \
                                                     KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char((_mem), 'H'))));  \
        ASSERT_OK(kefir_ast_generic_selection_append((_mem), selection1, type_name4,                                   \
                                                     KEFIR_AST_NODE_BASE(kefir_ast_new_constant_uint((_mem), 1556)))); \
        ASSERT_OK(kefir_ast_generic_selection_append(                                                                  \
            (_mem), selection1, NULL, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_double((_mem), 2.71))));              \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(selection1)));                        \
        ASSERT(KEFIR_AST_TYPE_SAME(selection1->base.properties.type, (_type)));                                        \
        ASSERT(selection1->base.properties.expression_props.lvalue == (_lvalue));                                      \
        ASSERT(selection1->base.properties.expression_props.constant_expression == (_const));                          \
        ASSERT(selection1->base.properties.expression_props.addressable == (_addressable));                            \
        ASSERT_OK(KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(selection1)));                                       \
    } while (0)

DEFINE_CASE(ast_node_analysis_generic_selections, "AST node analysis - generic selections") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "x", kefir_ast_type_char(), NULL, NULL,
                                                       NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_define_constant(&kft_mem, &local_context, "y",
                                                      &KEFIR_AST_CONSTANT_EXPRESSION_INT_VALUE(100),
                                                      kefir_ast_type_unsigned_int(), NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "z", kefir_ast_type_double(), NULL,
                                                       NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "w", kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_void()),
        NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "q", kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_char()),
        NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "r", kefir_ast_type_signed_long_long(),
                                                       NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "bool", kefir_ast_type_boolean(), NULL,
                                                       NULL, NULL, NULL));

    ASSERT_GENERIC_SELECTION(&kft_mem, context, "x", kefir_ast_type_boolean(), true, false, true);
    ASSERT_GENERIC_SELECTION(&kft_mem, context, "y", kefir_ast_type_float(), false, true, false);
    ASSERT_GENERIC_SELECTION(&kft_mem, context, "z", kefir_ast_type_signed_int(), false, true, false);
    ASSERT_GENERIC_SELECTION(&kft_mem, context, "w", kefir_ast_type_unsigned_int(), false, true, false);
    ASSERT_GENERIC_SELECTION(&kft_mem, context, "q", kefir_ast_type_double(), false, true, false);
    ASSERT_GENERIC_SELECTION(&kft_mem, context, "r", kefir_ast_type_double(), false, true, false);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_GENERIC_SELECTION

#define ASSERT_CAST(_mem, _context, _type, _type_name, _expr, _const)                                      \
    do {                                                                                                   \
        struct kefir_ast_cast_operator *oper = kefir_ast_new_cast_operator((_mem), (_type_name), (_expr)); \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(oper)));                  \
        ASSERT(oper->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                      \
        ASSERT(KEFIR_AST_TYPE_SAME(oper->base.properties.type, (_type)));                                  \
        ASSERT(oper->base.properties.expression_props.constant_expression == (_const));                    \
        ASSERT(!oper->base.properties.expression_props.lvalue);                                            \
        ASSERT(!oper->base.properties.expression_props.addressable);                                       \
        ASSERT(!oper->base.properties.expression_props.bitfield_props.bitfield);                           \
        ASSERT_OK(KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(oper)));                                 \
    } while (0)

DEFINE_CASE(ast_node_analysis_cast_operators, "AST node analysis - cast operators") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_struct_type *structure1 = NULL;
    const struct kefir_ast_type *structure1_type =
        kefir_ast_type_structure(&kft_mem, context->type_bundle, "", &structure1);

    ASSERT_OK(kefir_ast_local_context_define_auto(&kft_mem, &local_context, "x", kefir_ast_type_float(), NULL, NULL,
                                                  NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_define_auto(&kft_mem, &local_context, "y", structure1_type, NULL, NULL, NULL,
                                                  NULL, NULL));

    struct kefir_ast_type_name *type_name1 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name1->type_decl.specifiers,
                                                         kefir_ast_type_specifier_int(&kft_mem)));
    ASSERT_CAST(&kft_mem, context, kefir_ast_type_signed_int(), type_name1,
                KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 'a')), true);

    struct kefir_ast_type_name *type_name2 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name2->type_decl.specifiers,
                                                         kefir_ast_type_specifier_float(&kft_mem)));
    ASSERT_CAST(&kft_mem, context, kefir_ast_type_float(), type_name2,
                KEFIR_AST_NODE_BASE(kefir_ast_new_constant_bool(&kft_mem, false)), true);

    struct kefir_ast_type_name *type_name3 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name3->type_decl.specifiers,
                                                         kefir_ast_type_specifier_long(&kft_mem)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name3->type_decl.specifiers,
                                                         kefir_ast_type_specifier_long(&kft_mem)));
    ASSERT_CAST(&kft_mem, context, kefir_ast_type_signed_long_long(), type_name3,
                KEFIR_AST_NODE_BASE(kefir_ast_new_constant_double(&kft_mem, 3.14)), true);

    struct kefir_ast_type_name *type_name4 = kefir_ast_new_type_name(
        &kft_mem, kefir_ast_declarator_pointer(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name4->type_decl.specifiers,
                                                         kefir_ast_type_specifier_char(&kft_mem)));
    ASSERT_CAST(&kft_mem, context, kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_char()),
                type_name4, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 0)), true);

    struct kefir_ast_type_name *type_name5 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name5->type_decl.specifiers,
                                                         kefir_ast_type_specifier_int(&kft_mem)));
    ASSERT_CAST(&kft_mem, context, kefir_ast_type_signed_int(), type_name5,
                KEFIR_AST_NODE_BASE(kefir_ast_new_unary_operation(
                    &kft_mem, KEFIR_AST_OPERATION_ADDRESS,
                    KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")))),
                false);

    struct kefir_ast_type_name *type_name6 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name6->type_decl.specifiers,
                                                         kefir_ast_type_specifier_void(&kft_mem)));
    ASSERT_CAST(&kft_mem, context, kefir_ast_type_void(), type_name6,
                KEFIR_AST_NODE_BASE(kefir_ast_new_unary_operation(
                    &kft_mem, KEFIR_AST_OPERATION_ADDRESS,
                    KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")))),
                false);

    struct kefir_ast_type_name *type_name7 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name7->type_decl.specifiers,
                                                         kefir_ast_type_specifier_float(&kft_mem)));
    struct kefir_ast_cast_operator *oper = kefir_ast_new_cast_operator(
        &kft_mem, type_name7,
        KEFIR_AST_NODE_BASE(kefir_ast_new_unary_operation(
            &kft_mem, KEFIR_AST_OPERATION_ADDRESS,
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")))));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    struct kefir_ast_type_name *type_name8 = kefir_ast_new_type_name(
        &kft_mem, kefir_ast_declarator_pointer(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name8->type_decl.specifiers,
                                                         kefir_ast_type_specifier_char(&kft_mem)));
    oper = kefir_ast_new_cast_operator(&kft_mem, type_name8,
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_constant_double(&kft_mem, 3.14)));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    struct kefir_ast_type_name *type_name9 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name9->type_decl.specifiers,
                                                         kefir_ast_type_specifier_short(&kft_mem)));
    oper = kefir_ast_new_cast_operator(&kft_mem, type_name9,
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_cast_operator_qualified_rvalue, "AST node analysis - cast to qualified rvalues") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_type_name *type_name1 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name1->type_decl.specifiers,
                                                         kefir_ast_type_specifier_int(&kft_mem)));
    ASSERT_CAST(&kft_mem, context, kefir_ast_type_signed_int(), type_name1,
                KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 0)), true);

    struct kefir_ast_type_name *type_name2 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name2->type_decl.specifiers,
                                                         kefir_ast_type_specifier_int(&kft_mem)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name2->type_decl.specifiers,
                                                         kefir_ast_type_qualifier_const(&kft_mem)));
    ASSERT_CAST(&kft_mem, context, kefir_ast_type_signed_int(), type_name2,
                KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 0)), true);

    struct kefir_ast_type_name *type_name3 =
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name3->type_decl.specifiers,
                                                         kefir_ast_type_specifier_int(&kft_mem)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name3->type_decl.specifiers,
                                                         kefir_ast_type_qualifier_const(&kft_mem)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name3->type_decl.specifiers,
                                                         kefir_ast_type_qualifier_volatile(&kft_mem)));
    ASSERT_CAST(&kft_mem, context, kefir_ast_type_signed_int(), type_name3,
                KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 0)), true);

    struct kefir_ast_type_name *type_name4 = kefir_ast_new_type_name(
        &kft_mem, kefir_ast_declarator_pointer(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name4->type_decl.specifiers,
                                                         kefir_ast_type_specifier_int(&kft_mem)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name4->type_decl.specifiers,
                                                         kefir_ast_type_qualifier_const(&kft_mem)));
    ASSERT_OK(kefir_ast_declarator_specifier_list_append(&kft_mem, &type_name4->type_decl.specifiers,
                                                         kefir_ast_type_qualifier_volatile(&kft_mem)));
    ASSERT_CAST(
        &kft_mem, context,
        kefir_ast_type_pointer(
            &kft_mem, context->type_bundle,
            kefir_ast_type_qualified(&kft_mem, context->type_bundle, kefir_ast_type_signed_int(),
                                     (struct kefir_ast_type_qualification) {.constant = true, .volatile_type = true})),
        type_name4, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 0)), true);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_CAST

#define ASSERT_BINARY(_mem, _context, _oper, _arg1, _arg2, _type, _const)                                            \
    do {                                                                                                             \
        struct kefir_ast_binary_operation *oper = kefir_ast_new_binary_operation((_mem), (_oper), (_arg1), (_arg2)); \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(oper)));                            \
        ASSERT(oper->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                                \
        ASSERT(KEFIR_AST_TYPE_SAME(oper->base.properties.type, (_type)));                                            \
        ASSERT(oper->base.properties.expression_props.constant_expression == (_const));                              \
        ASSERT(!oper->base.properties.expression_props.lvalue);                                                      \
        ASSERT(!oper->base.properties.expression_props.addressable);                                                 \
        ASSERT(!oper->base.properties.expression_props.bitfield_props.bitfield);                                     \
        ASSERT_OK(KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(oper)));                                           \
    } while (0)

struct kefir_ast_constant *make_constant(struct kefir_mem *, const struct kefir_ast_type *);
struct kefir_ast_constant *make_constant2(struct kefir_mem *, const struct kefir_ast_type *, kefir_int64_t);

DEFINE_CASE(ast_node_analysis_multiplicative_operators, "AST node analysis - multiplicative operators") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_global_context_define_static(&kft_mem, &global_context, "x", kefir_ast_type_signed_int(), NULL,
                                                     NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_global_context_define_constant(&kft_mem, &global_context, "y",
                                                       &KEFIR_AST_CONSTANT_EXPRESSION_INT_VALUE(100),
                                                       type_traits->underlying_enumeration_type, NULL, NULL));

#define MULDIV(_oper)                                                                                                \
    ASSERT_BINARY(&kft_mem, context, (_oper),                                                                        \
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),                    \
                  KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 2)), kefir_ast_type_signed_int(), false); \
                                                                                                                     \
    ASSERT_BINARY(&kft_mem, context, (_oper), KEFIR_AST_NODE_BASE(kefir_ast_new_constant_long(&kft_mem, 3)),         \
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),                    \
                  kefir_ast_type_signed_long(), false);                                                              \
                                                                                                                     \
    ASSERT_BINARY(&kft_mem, context, (_oper),                                                                        \
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),                    \
                  KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 4)), kefir_ast_type_signed_int(), true);  \
                                                                                                                     \
    ASSERT_BINARY(&kft_mem, context, (_oper), KEFIR_AST_NODE_BASE(kefir_ast_new_constant_long(&kft_mem, 5)),         \
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),                    \
                  kefir_ast_type_signed_long(), true);

    MULDIV(KEFIR_AST_OPERATION_MULTIPLY)
    MULDIV(KEFIR_AST_OPERATION_DIVIDE)

    const struct kefir_ast_type *TYPES[] = {
        kefir_ast_type_boolean(),          kefir_ast_type_char(),         kefir_ast_type_unsigned_char(),
        kefir_ast_type_signed_char(),      kefir_ast_type_unsigned_int(), kefir_ast_type_signed_int(),
        kefir_ast_type_unsigned_long(),    kefir_ast_type_signed_long(),  kefir_ast_type_unsigned_long_long(),
        kefir_ast_type_signed_long_long(), kefir_ast_type_float(),        kefir_ast_type_double()};
    const kefir_size_t TYPES_LEN = sizeof(TYPES) / sizeof(TYPES[0]);

    for (kefir_size_t i = 0; i < TYPES_LEN; i++) {
        for (kefir_size_t j = 0; j < TYPES_LEN; j++) {
            ASSERT_BINARY(
                &kft_mem, context, KEFIR_AST_OPERATION_MULTIPLY, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])),
                kefir_ast_type_common_arithmetic(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                 TYPES[j], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                true);
            ASSERT_BINARY(
                &kft_mem, context, KEFIR_AST_OPERATION_DIVIDE, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])),
                kefir_ast_type_common_arithmetic(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                 TYPES[j], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                true);
            if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(TYPES[i]) && KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(TYPES[j])) {
                ASSERT_BINARY(
                    &kft_mem, context, KEFIR_AST_OPERATION_MODULO,
                    KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                    KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])),
                    kefir_ast_type_common_arithmetic(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                     TYPES[j], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                    true);
            }
        }
    }

#undef MULDIV

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_MODULO,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 11)), kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_MODULO,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_constant_ulong(&kft_mem, 12)),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  kefir_ast_type_unsigned_long(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_MODULO,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_constant_char(&kft_mem, 11)), kefir_ast_type_signed_int(), true);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_MODULO,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_constant_ulong(&kft_mem, 12)),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_unsigned_long(), true);

    struct kefir_ast_binary_operation *oper = kefir_ast_new_binary_operation(
        &kft_mem, KEFIR_AST_OPERATION_MODULO, KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 105)),
        KEFIR_AST_NODE_BASE(kefir_ast_new_constant_float(&kft_mem, 11.0f)));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    oper = kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_MODULO,
                                          KEFIR_AST_NODE_BASE(kefir_ast_new_constant_double(&kft_mem, 106.0f)),
                                          KEFIR_AST_NODE_BASE(kefir_ast_new_constant_long_long(&kft_mem, 12)));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_add_operator, "AST node analysis - add operator") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "x",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_short()), NULL, NULL, NULL,
        NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "y", kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_void()),
        NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_global_context_define_constant(&kft_mem, &global_context, "X",
                                                       &KEFIR_AST_CONSTANT_EXPRESSION_INT_VALUE(100),
                                                       type_traits->underlying_enumeration_type, NULL, NULL));

    const struct kefir_ast_type *TYPES[] = {
        kefir_ast_type_boolean(),          kefir_ast_type_char(),         kefir_ast_type_unsigned_char(),
        kefir_ast_type_signed_char(),      kefir_ast_type_unsigned_int(), kefir_ast_type_signed_int(),
        kefir_ast_type_unsigned_long(),    kefir_ast_type_signed_long(),  kefir_ast_type_unsigned_long_long(),
        kefir_ast_type_signed_long_long(), kefir_ast_type_float(),        kefir_ast_type_double()};
    const kefir_size_t TYPES_LEN = sizeof(TYPES) / sizeof(TYPES[0]);

    for (kefir_size_t i = 0; i < TYPES_LEN; i++) {
        for (kefir_size_t j = 0; j < TYPES_LEN; j++) {
            ASSERT_BINARY(
                &kft_mem, context, KEFIR_AST_OPERATION_ADD, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])),
                kefir_ast_type_common_arithmetic(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                 TYPES[j], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                true);
        }

        ASSERT_BINARY(
            &kft_mem, context, KEFIR_AST_OPERATION_ADD, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
            kefir_ast_type_common_arithmetic(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                             type_traits->underlying_enumeration_type,
                                             KEFIR_AST_BITFIELD_PROPERTIES_NONE),
            true);

        if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(TYPES[i])) {
            ASSERT_BINARY(
                &kft_mem, context, KEFIR_AST_OPERATION_ADD, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_short()), false);

            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_ADD,
                          KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_short()),
                          false);

            struct kefir_ast_binary_operation *oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_ADD, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));
        } else {
            struct kefir_ast_binary_operation *oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_ADD, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));
        }
    }

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_subtract_operator, "AST node analysis - subtraction operator") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "x",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_short()), NULL, NULL, NULL,
        NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "y", kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_void()),
        NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_global_context_define_constant(&kft_mem, &global_context, "X",
                                                       &KEFIR_AST_CONSTANT_EXPRESSION_INT_VALUE(100),
                                                       type_traits->underlying_enumeration_type, NULL, NULL));

    const struct kefir_ast_type *TYPES[] = {
        kefir_ast_type_boolean(),          kefir_ast_type_char(),         kefir_ast_type_unsigned_char(),
        kefir_ast_type_signed_char(),      kefir_ast_type_unsigned_int(), kefir_ast_type_signed_int(),
        kefir_ast_type_unsigned_long(),    kefir_ast_type_signed_long(),  kefir_ast_type_unsigned_long_long(),
        kefir_ast_type_signed_long_long(), kefir_ast_type_float(),        kefir_ast_type_double()};
    const kefir_size_t TYPES_LEN = sizeof(TYPES) / sizeof(TYPES[0]);

    for (kefir_size_t i = 0; i < TYPES_LEN; i++) {
        for (kefir_size_t j = 0; j < TYPES_LEN; j++) {
            ASSERT_BINARY(
                &kft_mem, context, KEFIR_AST_OPERATION_SUBTRACT, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])),
                kefir_ast_type_common_arithmetic(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                 TYPES[j], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                true);
        }

        ASSERT_BINARY(
            &kft_mem, context, KEFIR_AST_OPERATION_SUBTRACT, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
            kefir_ast_type_common_arithmetic(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                             type_traits->underlying_enumeration_type,
                                             KEFIR_AST_BITFIELD_PROPERTIES_NONE),
            true);

        if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(TYPES[i])) {
            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_SUBTRACT,
                          KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_short()),
                          false);

            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_SUBTRACT,
                          KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_short()),
                          false);

            struct kefir_ast_binary_operation *oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_SUBTRACT, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

            oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_SUBTRACT,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])));
            ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));
        } else {
            struct kefir_ast_binary_operation *oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_SUBTRACT, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));
        }
    }

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_SUBTRACT,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  type_traits->ptrdiff_type, false);

    struct kefir_ast_binary_operation *oper =
        kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_SUBTRACT,
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_shift_operator, "AST node analysis - shift operators") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "x", kefir_ast_type_signed_short(),
                                                       NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_global_context_define_constant(&kft_mem, &global_context, "X",
                                                       &KEFIR_AST_CONSTANT_EXPRESSION_INT_VALUE(7),
                                                       type_traits->underlying_enumeration_type, NULL, NULL));

    const struct kefir_ast_type *TYPES[] = {
        kefir_ast_type_boolean(),         kefir_ast_type_char(),         kefir_ast_type_unsigned_char(),
        kefir_ast_type_signed_char(),     kefir_ast_type_unsigned_int(), kefir_ast_type_signed_int(),
        kefir_ast_type_unsigned_long(),   kefir_ast_type_signed_long(),  kefir_ast_type_unsigned_long_long(),
        kefir_ast_type_signed_long_long()};
    const kefir_size_t TYPES_LEN = sizeof(TYPES) / sizeof(TYPES[0]);

    for (kefir_size_t i = 0; i < TYPES_LEN; i++) {
        for (kefir_size_t j = 0; j < TYPES_LEN; j++) {
            ASSERT_BINARY(
                &kft_mem, context, KEFIR_AST_OPERATION_SHIFT_LEFT,
                KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[i], 1000)),
                KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[j], 6)),
                kefir_ast_type_int_promotion(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE), true);

            ASSERT_BINARY(
                &kft_mem, context, KEFIR_AST_OPERATION_SHIFT_RIGHT,
                KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[i], 2000)),
                KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[j], 5)),
                kefir_ast_type_int_promotion(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE), true);
        }

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_SHIFT_RIGHT,
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                      KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[i], 7)),
                      kefir_ast_type_int_promotion(context->type_traits, kefir_ast_type_signed_short(),
                                                   KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                      false);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_SHIFT_RIGHT,
                      KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[i], 3)),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                      kefir_ast_type_int_promotion(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                      false);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_SHIFT_LEFT,
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                      KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[i], 2)),
                      kefir_ast_type_int_promotion(context->type_traits, kefir_ast_type_signed_short(),
                                                   KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                      false);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_SHIFT_LEFT,
                      KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[i], 127)),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                      kefir_ast_type_int_promotion(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                      false);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_SHIFT_RIGHT,
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                      KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[i], 10)),
                      kefir_ast_type_int_promotion(context->type_traits, kefir_ast_type_signed_short(),
                                                   KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                      true);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_SHIFT_RIGHT,
                      KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[i], -13)),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                      kefir_ast_type_int_promotion(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                      true);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_SHIFT_LEFT,
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                      KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[i], 8)),
                      kefir_ast_type_int_promotion(context->type_traits, kefir_ast_type_signed_short(),
                                                   KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                      true);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_SHIFT_LEFT,
                      KEFIR_AST_NODE_BASE(make_constant2(&kft_mem, TYPES[i], 13)),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                      kefir_ast_type_int_promotion(context->type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                      true);

        struct kefir_ast_binary_operation *oper = kefir_ast_new_binary_operation(
            &kft_mem, KEFIR_AST_OPERATION_SHIFT_LEFT, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
            KEFIR_AST_NODE_BASE(make_constant(&kft_mem, kefir_ast_type_float())));
        ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
        ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

        oper = kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_SHIFT_RIGHT,
                                              KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                                              KEFIR_AST_NODE_BASE(make_constant(&kft_mem, kefir_ast_type_float())));
        ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
        ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));
    }

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_relational_operators, "AST node analysis - relational operators") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "x", kefir_ast_type_signed_int(), NULL,
                                                       NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "y",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_int()), NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "z", kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_void()),
        NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_global_context_define_constant(&kft_mem, &global_context, "X",
                                                       &KEFIR_AST_CONSTANT_EXPRESSION_INT_VALUE(100),
                                                       type_traits->underlying_enumeration_type, NULL, NULL));

    const struct kefir_ast_type *TYPES[] = {
        kefir_ast_type_boolean(),          kefir_ast_type_char(),         kefir_ast_type_unsigned_char(),
        kefir_ast_type_signed_char(),      kefir_ast_type_unsigned_int(), kefir_ast_type_signed_int(),
        kefir_ast_type_unsigned_long(),    kefir_ast_type_signed_long(),  kefir_ast_type_unsigned_long_long(),
        kefir_ast_type_signed_long_long(), kefir_ast_type_float(),        kefir_ast_type_double()};
    const kefir_size_t TYPES_LEN = sizeof(TYPES) / sizeof(TYPES[0]);

    for (kefir_size_t i = 0; i < TYPES_LEN; i++) {
        for (kefir_size_t j = 0; j < TYPES_LEN; j++) {
            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LESS,
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])), kefir_ast_type_signed_int(), true);

            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LESS_EQUAL,
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])), kefir_ast_type_signed_int(), true);

            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_GREATER,
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])), kefir_ast_type_signed_int(), true);

            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_GREATER_EQUAL,
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])), kefir_ast_type_signed_int(), true);
        }

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LESS,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                      kefir_ast_type_signed_int(), false);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LESS_EQUAL,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                      kefir_ast_type_signed_int(), false);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_GREATER,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                      kefir_ast_type_signed_int(), false);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_GREATER_EQUAL,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                      kefir_ast_type_signed_int(), false);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LESS,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                      kefir_ast_type_signed_int(), true);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LESS_EQUAL,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                      kefir_ast_type_signed_int(), true);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_GREATER,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                      kefir_ast_type_signed_int(), true);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_GREATER_EQUAL,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                      kefir_ast_type_signed_int(), true);
    }

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LESS,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LESS,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LESS_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LESS_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_GREATER,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_GREATER,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_GREATER_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_GREATER_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  kefir_ast_type_signed_int(), false);

    struct kefir_ast_binary_operation *oper =
        kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_LESS,
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    oper =
        kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_LESS_EQUAL,
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    oper =
        kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_GREATER,
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    oper =
        kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_GREATER_EQUAL,
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")));
    ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_equality_operators, "AST node analysis - equality operators") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "x",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_signed_int()), NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "y",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_int()), NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "z", kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_void()),
        NULL, NULL, NULL, NULL));

    const struct kefir_ast_type *TYPES[] = {
        kefir_ast_type_boolean(),          kefir_ast_type_char(),         kefir_ast_type_unsigned_char(),
        kefir_ast_type_signed_char(),      kefir_ast_type_unsigned_int(), kefir_ast_type_signed_int(),
        kefir_ast_type_unsigned_long(),    kefir_ast_type_signed_long(),  kefir_ast_type_unsigned_long_long(),
        kefir_ast_type_signed_long_long(), kefir_ast_type_float(),        kefir_ast_type_double()};
    const kefir_size_t TYPES_LEN = sizeof(TYPES) / sizeof(TYPES[0]);

    for (kefir_size_t i = 0; i < TYPES_LEN; i++) {
        for (kefir_size_t j = 0; j < TYPES_LEN; j++) {
            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_EQUAL,
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])), kefir_ast_type_signed_int(), true);

            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_NOT_EQUAL,
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])), kefir_ast_type_signed_int(), true);
        }
        if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(TYPES[i])) {
            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_EQUAL,
                          KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])), kefir_ast_type_signed_int(), false);

            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_NOT_EQUAL,
                          KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])), kefir_ast_type_signed_int(), false);
        } else {
            struct kefir_ast_binary_operation *oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_EQUAL,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

            oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_NOT_EQUAL,
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));
        }
    }

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_NOT_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_NOT_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_NOT_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_NOT_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_NOT_EQUAL,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "z")),
                  kefir_ast_type_signed_int(), false);

    struct kefir_ast_binary_operation *oper =
        kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_EQUAL,
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    oper =
        kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_EQUAL,
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    oper =
        kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_NOT_EQUAL,
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    oper =
        kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_NOT_EQUAL,
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                                       KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")));
    ASSERT_OK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
    ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_bitwise_operators, "AST node analysis - bitwise operators") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "x", kefir_ast_type_signed_int(), NULL,
                                                       NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "y",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_signed_int()), NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_global_context_define_constant(&kft_mem, &global_context, "X",
                                                       &KEFIR_AST_CONSTANT_EXPRESSION_INT_VALUE(100),
                                                       type_traits->underlying_enumeration_type, NULL, NULL));

    const struct kefir_ast_type *TYPES[] = {
        kefir_ast_type_boolean(),          kefir_ast_type_char(),         kefir_ast_type_unsigned_char(),
        kefir_ast_type_signed_char(),      kefir_ast_type_unsigned_int(), kefir_ast_type_signed_int(),
        kefir_ast_type_unsigned_long(),    kefir_ast_type_signed_long(),  kefir_ast_type_unsigned_long_long(),
        kefir_ast_type_signed_long_long(), kefir_ast_type_float(),        kefir_ast_type_double()};
    const kefir_size_t TYPES_LEN = sizeof(TYPES) / sizeof(TYPES[0]);

    for (kefir_size_t i = 0; i < TYPES_LEN; i++) {
        for (kefir_size_t j = 0; j < TYPES_LEN; j++) {
            if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(TYPES[i]) && KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(TYPES[j])) {
                ASSERT_BINARY(
                    &kft_mem, context, KEFIR_AST_OPERATION_BITWISE_AND,
                    KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                    KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])),
                    kefir_ast_type_common_arithmetic(type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                     TYPES[j], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                    true);

                ASSERT_BINARY(
                    &kft_mem, context, KEFIR_AST_OPERATION_BITWISE_OR,
                    KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                    KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])),
                    kefir_ast_type_common_arithmetic(type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                     TYPES[j], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                    true);

                ASSERT_BINARY(
                    &kft_mem, context, KEFIR_AST_OPERATION_BITWISE_XOR,
                    KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                    KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])),
                    kefir_ast_type_common_arithmetic(type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                     TYPES[j], KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                    true);
            } else {
                struct kefir_ast_binary_operation *oper = kefir_ast_new_binary_operation(
                    &kft_mem, KEFIR_AST_OPERATION_BITWISE_AND, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                    KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])));
                ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
                ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

                oper = kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_BITWISE_OR,
                                                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                                                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])));
                ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
                ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

                oper = kefir_ast_new_binary_operation(&kft_mem, KEFIR_AST_OPERATION_BITWISE_XOR,
                                                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                                                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])));
                ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
                ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));
            }
        }

        if (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(TYPES[i])) {
            ASSERT_BINARY(
                &kft_mem, context, KEFIR_AST_OPERATION_BITWISE_AND,
                KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                kefir_ast_type_common_arithmetic(type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                 kefir_ast_type_signed_int(), KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                false);

            ASSERT_BINARY(
                &kft_mem, context, KEFIR_AST_OPERATION_BITWISE_OR,
                KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                kefir_ast_type_common_arithmetic(type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                 kefir_ast_type_signed_int(), KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                false);

            ASSERT_BINARY(
                &kft_mem, context, KEFIR_AST_OPERATION_BITWISE_XOR,
                KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                kefir_ast_type_common_arithmetic(type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                 kefir_ast_type_signed_int(), KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                false);

            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_BITWISE_AND,
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                          kefir_ast_type_common_arithmetic(type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                           type_traits->underlying_enumeration_type,
                                                           KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                          true);

            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_BITWISE_OR,
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                          kefir_ast_type_common_arithmetic(type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                           type_traits->underlying_enumeration_type,
                                                           KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                          true);

            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_BITWISE_XOR,
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                          kefir_ast_type_common_arithmetic(type_traits, TYPES[i], KEFIR_AST_BITFIELD_PROPERTIES_NONE,
                                                           type_traits->underlying_enumeration_type,
                                                           KEFIR_AST_BITFIELD_PROPERTIES_NONE),
                          true);
        } else {
            struct kefir_ast_binary_operation *oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_BITWISE_AND, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

            oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_BITWISE_OR, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

            oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_BITWISE_XOR, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

            oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_BITWISE_AND, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

            oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_BITWISE_OR, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

            oper = kefir_ast_new_binary_operation(
                &kft_mem, KEFIR_AST_OPERATION_BITWISE_XOR, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")));
            ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
            ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));
        }

        struct kefir_ast_binary_operation *oper = kefir_ast_new_binary_operation(
            &kft_mem, KEFIR_AST_OPERATION_BITWISE_AND, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")));
        ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
        ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

        oper = kefir_ast_new_binary_operation(
            &kft_mem, KEFIR_AST_OPERATION_BITWISE_OR, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")));
        ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
        ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));

        oper = kefir_ast_new_binary_operation(
            &kft_mem, KEFIR_AST_OPERATION_BITWISE_XOR, KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")));
        ASSERT_NOK(kefir_ast_analyze_node(&kft_mem, context, KEFIR_AST_NODE_BASE(oper)));
        ASSERT_OK(KEFIR_AST_NODE_FREE(&kft_mem, KEFIR_AST_NODE_BASE(oper)));
    }

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_logical_operators, "AST node analysis - logical operators") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "x", kefir_ast_type_signed_int(), NULL,
                                                       NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "y",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_signed_int()), NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_global_context_define_constant(&kft_mem, &global_context, "X",
                                                       &KEFIR_AST_CONSTANT_EXPRESSION_INT_VALUE(100),
                                                       type_traits->underlying_enumeration_type, NULL, NULL));

    const struct kefir_ast_type *TYPES[] = {
        kefir_ast_type_boolean(),          kefir_ast_type_char(),         kefir_ast_type_unsigned_char(),
        kefir_ast_type_signed_char(),      kefir_ast_type_unsigned_int(), kefir_ast_type_signed_int(),
        kefir_ast_type_unsigned_long(),    kefir_ast_type_signed_long(),  kefir_ast_type_unsigned_long_long(),
        kefir_ast_type_signed_long_long(), kefir_ast_type_float(),        kefir_ast_type_double()};
    const kefir_size_t TYPES_LEN = sizeof(TYPES) / sizeof(TYPES[0]);

    for (kefir_size_t i = 0; i < TYPES_LEN; i++) {
        for (kefir_size_t j = 0; j < TYPES_LEN; j++) {
            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_AND,
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])), kefir_ast_type_signed_int(), true);
            ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_OR,
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                          KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[j])), kefir_ast_type_signed_int(), true);
        }

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_AND,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                      kefir_ast_type_signed_int(), false);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_AND,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                      kefir_ast_type_signed_int(), false);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_AND,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                      kefir_ast_type_signed_int(), true);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_OR,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                      kefir_ast_type_signed_int(), true);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_OR,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                      kefir_ast_type_signed_int(), true);

        ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_OR,
                      KEFIR_AST_NODE_BASE(make_constant(&kft_mem, TYPES[i])),
                      KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                      kefir_ast_type_signed_int(), true);
    }

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_AND,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_AND,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_AND,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_AND,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_AND,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                  kefir_ast_type_signed_int(), true);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_OR,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_OR,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_OR,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "y")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_OR,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "x")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                  kefir_ast_type_signed_int(), false);

    ASSERT_BINARY(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_OR,
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                  KEFIR_AST_NODE_BASE(kefir_ast_new_identifier(&kft_mem, context->symbols, "X")),
                  kefir_ast_type_signed_int(), true);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_BINARY

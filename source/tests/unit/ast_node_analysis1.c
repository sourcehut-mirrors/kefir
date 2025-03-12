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
#include "declarator_analysis.h"

#define ASSERT_CONSTANT(_mem, _context, _cnst, _const_type)                                 \
    do {                                                                                    \
        struct kefir_ast_constant *const1 = (_cnst);                                        \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(const1))); \
        ASSERT(const1->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);     \
        ASSERT(KEFIR_AST_TYPE_SAME(const1->base.properties.type, (_const_type)));           \
        ASSERT(const1->base.properties.expression_props.constant_expression);               \
        ASSERT(!const1->base.properties.expression_props.lvalue);                           \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(const1));                           \
    } while (0)

DEFINE_CASE(ast_node_analysis_constants, "AST node analysis - constant types") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_bool(&kft_mem, false), kefir_ast_type_boolean());
    ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_bool(&kft_mem, true), kefir_ast_type_boolean());

    for (kefir_char_t i = KEFIR_CHAR_MIN; i < KEFIR_CHAR_MAX; i++) {
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_char(&kft_mem, i), kefir_ast_type_signed_int());
    }

    for (kefir_char32_t i = KEFIR_CHAR32_MIN; i < 4096; i++) {
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_wide_char(&kft_mem, i), type_traits->wide_char_type);
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_unicode16_char(&kft_mem, i),
                        type_traits->unicode16_char_type);
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_unicode32_char(&kft_mem, i),
                        type_traits->unicode32_char_type);
    }

    for (kefir_int_t i = -1000; i < 1000; i++) {
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_int(&kft_mem, i), kefir_ast_type_signed_int());
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_long(&kft_mem, i), kefir_ast_type_signed_long());
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_long_long(&kft_mem, i),
                        kefir_ast_type_signed_long_long());
    }

    for (kefir_uint_t i = 0; i < 10000; i++) {
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_uint(&kft_mem, i), kefir_ast_type_unsigned_int());
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_ulong(&kft_mem, i), kefir_ast_type_unsigned_long());
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_ulong_long(&kft_mem, i),
                        kefir_ast_type_unsigned_long_long());
    }

    for (kefir_float32_t f = -100.0f; f < 100.0f; f += 0.01f) {
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_float(&kft_mem, f), kefir_ast_type_float());
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_double(&kft_mem, (kefir_float64_t) f),
                        kefir_ast_type_double());
        ASSERT_CONSTANT(&kft_mem, context, kefir_ast_new_constant_long_double(&kft_mem, (kefir_long_double_t) f),
                        kefir_ast_type_long_double());
    }

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_CONSTANT

#define ASSERT_STRING_LITERAL(_mem, _context, _type, _builder, _literal, _underlying, _literal_type)                 \
    do {                                                                                                             \
        const _type LITERAL[] = _literal;                                                                            \
        struct kefir_ast_string_literal *literal = _builder((_mem), LITERAL, sizeof(LITERAL));                       \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(literal)));                         \
        ASSERT(literal->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                             \
        ASSERT(KEFIR_AST_TYPE_SAME(                                                                                  \
            literal->base.properties.type,                                                                           \
            kefir_ast_type_array((_mem), (_context)->type_bundle, (_underlying),                                     \
                                 kefir_ast_constant_expression_integer((_mem), sizeof(LITERAL)), NULL)));            \
        ASSERT(literal->base.properties.expression_props.constant_expression);                                       \
        ASSERT(!literal->base.properties.expression_props.lvalue);                                                   \
        ASSERT(literal->base.properties.expression_props.string_literal.type == (_literal_type));                    \
        ASSERT(literal->base.properties.expression_props.string_literal.length == sizeof(LITERAL));                  \
        ASSERT(memcmp(literal->base.properties.expression_props.string_literal.content, LITERAL, sizeof(LITERAL)) == \
               0);                                                                                                   \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(literal));                                                   \
    } while (0)

DEFINE_CASE(ast_node_analysis_string_literals_multibyte, "AST node analysis - multibyte string literals") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_multibyte, "", kefir_ast_type_char(),
                          KEFIR_AST_STRING_LITERAL_MULTIBYTE);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_multibyte, "1", kefir_ast_type_char(),
                          KEFIR_AST_STRING_LITERAL_MULTIBYTE);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_multibyte, "abc", kefir_ast_type_char(),
                          KEFIR_AST_STRING_LITERAL_MULTIBYTE);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_multibyte, "Hello, world!",
                          kefir_ast_type_char(), KEFIR_AST_STRING_LITERAL_MULTIBYTE);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_multibyte, "\0", kefir_ast_type_char(),
                          KEFIR_AST_STRING_LITERAL_MULTIBYTE);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_multibyte, "\0\0\0\t",
                          kefir_ast_type_char(), KEFIR_AST_STRING_LITERAL_MULTIBYTE);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_multibyte, "\n\n\n\taaa",
                          kefir_ast_type_char(), KEFIR_AST_STRING_LITERAL_MULTIBYTE);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_multibyte,
                          "    Hello,\n\tcruel\n\n\n  \t world\n!", kefir_ast_type_char(),
                          KEFIR_AST_STRING_LITERAL_MULTIBYTE);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_string_literals_unicode8, "AST node analysis - unicode8 string literals") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_unicode8, u8"", kefir_ast_type_char(),
                          KEFIR_AST_STRING_LITERAL_UNICODE8);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_unicode8, u8"1", kefir_ast_type_char(),
                          KEFIR_AST_STRING_LITERAL_UNICODE8);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_unicode8, u8"abc",
                          kefir_ast_type_char(), KEFIR_AST_STRING_LITERAL_UNICODE8);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_unicode8, u8"Hello, world!",
                          kefir_ast_type_char(), KEFIR_AST_STRING_LITERAL_UNICODE8);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_unicode8, u8"\0", kefir_ast_type_char(),
                          KEFIR_AST_STRING_LITERAL_UNICODE8);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_unicode8, u8"\0\0\0\t",
                          kefir_ast_type_char(), KEFIR_AST_STRING_LITERAL_UNICODE8);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_unicode8, u8"\n\n\n\taaa",
                          kefir_ast_type_char(), KEFIR_AST_STRING_LITERAL_UNICODE8);
    ASSERT_STRING_LITERAL(&kft_mem, context, char, kefir_ast_new_string_literal_unicode8,
                          u8"    Hello,\n\tcruel\n\n\n  \t world\n!", kefir_ast_type_char(),
                          KEFIR_AST_STRING_LITERAL_UNICODE8);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_string_literals_unicode16, "AST node analysis - unicode16 string literals") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char16_t, kefir_ast_new_string_literal_unicode16, u"",
                          type_traits->unicode16_char_type, KEFIR_AST_STRING_LITERAL_UNICODE16);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char16_t, kefir_ast_new_string_literal_unicode16, u"1",
                          type_traits->unicode16_char_type, KEFIR_AST_STRING_LITERAL_UNICODE16);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char16_t, kefir_ast_new_string_literal_unicode16, u"abc",
                          type_traits->unicode16_char_type, KEFIR_AST_STRING_LITERAL_UNICODE16);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char16_t, kefir_ast_new_string_literal_unicode16, u"Hello, world!",
                          type_traits->unicode16_char_type, KEFIR_AST_STRING_LITERAL_UNICODE16);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char16_t, kefir_ast_new_string_literal_unicode16, u"\0",
                          type_traits->unicode16_char_type, KEFIR_AST_STRING_LITERAL_UNICODE16);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char16_t, kefir_ast_new_string_literal_unicode16, u"\0\0\0\t",
                          type_traits->unicode16_char_type, KEFIR_AST_STRING_LITERAL_UNICODE16);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char16_t, kefir_ast_new_string_literal_unicode16, u"\n\n\n\taaa",
                          type_traits->unicode16_char_type, KEFIR_AST_STRING_LITERAL_UNICODE16);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char16_t, kefir_ast_new_string_literal_unicode16,
                          u"    Hello,\n\tcruel\n\n\n  \t world\n!", type_traits->unicode16_char_type,
                          KEFIR_AST_STRING_LITERAL_UNICODE16);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_string_literals_unicode32, "AST node analysis - unicode32 string literals") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char32_t, kefir_ast_new_string_literal_unicode32, U"",
                          type_traits->unicode32_char_type, KEFIR_AST_STRING_LITERAL_UNICODE32);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char32_t, kefir_ast_new_string_literal_unicode32, U"1",
                          type_traits->unicode32_char_type, KEFIR_AST_STRING_LITERAL_UNICODE32);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char32_t, kefir_ast_new_string_literal_unicode32, U"abc",
                          type_traits->unicode32_char_type, KEFIR_AST_STRING_LITERAL_UNICODE32);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char32_t, kefir_ast_new_string_literal_unicode32, U"Hello, world!",
                          type_traits->unicode32_char_type, KEFIR_AST_STRING_LITERAL_UNICODE32);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char32_t, kefir_ast_new_string_literal_unicode32, U"\0",
                          type_traits->unicode32_char_type, KEFIR_AST_STRING_LITERAL_UNICODE32);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char32_t, kefir_ast_new_string_literal_unicode32, U"\0\0\0\t",
                          type_traits->unicode32_char_type, KEFIR_AST_STRING_LITERAL_UNICODE32);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char32_t, kefir_ast_new_string_literal_unicode32, U"\n\n\n\taaa",
                          type_traits->unicode32_char_type, KEFIR_AST_STRING_LITERAL_UNICODE32);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_char32_t, kefir_ast_new_string_literal_unicode32,
                          U"    Hello,\n\tcruel\n\n\n  \t world\n!", type_traits->unicode32_char_type,
                          KEFIR_AST_STRING_LITERAL_UNICODE32);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_string_literals_wide, "AST node analysis - wide string literals") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_wchar_t, kefir_ast_new_string_literal_wide, L"",
                          type_traits->wide_char_type, KEFIR_AST_STRING_LITERAL_WIDE);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_wchar_t, kefir_ast_new_string_literal_wide, L"1",
                          type_traits->wide_char_type, KEFIR_AST_STRING_LITERAL_WIDE);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_wchar_t, kefir_ast_new_string_literal_wide, L"abc",
                          type_traits->wide_char_type, KEFIR_AST_STRING_LITERAL_WIDE);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_wchar_t, kefir_ast_new_string_literal_wide, L"Hello, world!",
                          type_traits->wide_char_type, KEFIR_AST_STRING_LITERAL_WIDE);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_wchar_t, kefir_ast_new_string_literal_wide, L"\0",
                          type_traits->wide_char_type, KEFIR_AST_STRING_LITERAL_WIDE);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_wchar_t, kefir_ast_new_string_literal_wide, L"\0\0\0\t",
                          type_traits->wide_char_type, KEFIR_AST_STRING_LITERAL_WIDE);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_wchar_t, kefir_ast_new_string_literal_wide, L"\n\n\n\taaa",
                          type_traits->wide_char_type, KEFIR_AST_STRING_LITERAL_WIDE);
    ASSERT_STRING_LITERAL(&kft_mem, context, kefir_wchar_t, kefir_ast_new_string_literal_wide,
                          L"    Hello,\n\tcruel\n\n\n  \t world\n!", type_traits->wide_char_type,
                          KEFIR_AST_STRING_LITERAL_WIDE);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_STRING_LITERAL

#define ASSERT_IDENTIFIER_LITERAL(_mem, _context, _identifier, _type, _constant, _lvalue)                         \
    do {                                                                                                          \
        struct kefir_ast_identifier *identifier =                                                                 \
            kefir_ast_new_identifier((_mem), (_context)->symbols, (_identifier));                                 \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(identifier)));                   \
        ASSERT(identifier->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                       \
        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_lvalue_conversion(identifier->base.properties.type), (_type))); \
        ASSERT(identifier->base.properties.expression_props.constant_expression == (_constant));                  \
        ASSERT(identifier->base.properties.expression_props.lvalue == (_lvalue));                                 \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(identifier));                                             \
    } while (0)

DEFINE_CASE(ast_node_analysis_identifiers, "AST node analysis - identifiers") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_function_type *function1 = NULL;
    const struct kefir_ast_type *function1_type =
        kefir_ast_type_function(&kft_mem, context->type_bundle, kefir_ast_type_void(), &function1);
    ASSERT_OK(kefir_ast_type_function_parameter(&kft_mem, context->type_bundle, function1,
                                                kefir_ast_type_unsigned_char(), NULL));

    struct kefir_ast_function_type *function2 = NULL;
    const struct kefir_ast_type *function2_type =
        kefir_ast_type_function(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_short(), &function2);
    ASSERT_OK(kefir_ast_type_function_parameter(&kft_mem, context->type_bundle, function2, NULL, NULL));

    ASSERT_OK(kefir_ast_global_context_declare_external(
        &kft_mem, &global_context, "var1",
        kefir_ast_type_qualified(
            &kft_mem, context->type_bundle, kefir_ast_type_signed_int(),
            (struct kefir_ast_type_qualification){.constant = true, .restricted = false, .volatile_type = false}),
        NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_global_context_declare_external(&kft_mem, &global_context, "var2", kefir_ast_type_float(), NULL,
                                                        NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_global_context_declare_function(&kft_mem, &global_context, KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                                                        true, "func1", function1_type, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_global_context_declare_function(&kft_mem, &global_context, KEFIR_AST_FUNCTION_SPECIFIER_INLINE,
                                                        true, "func2", function2_type, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_define_constant(&kft_mem, &local_context, "X",
                                                      kefir_ast_constant_expression_integer(&kft_mem, 100),
                                                      context->type_traits->underlying_enumeration_type, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_define_constant(&kft_mem, &local_context, "Y",
                                                      kefir_ast_constant_expression_integer(&kft_mem, -150),
                                                      context->type_traits->underlying_enumeration_type, NULL, NULL));

    ASSERT_IDENTIFIER_LITERAL(&kft_mem, context, "var1", kefir_ast_type_signed_int(), false, true);
    ASSERT_IDENTIFIER_LITERAL(&kft_mem, context, "var2", kefir_ast_type_float(), false, true);
    ASSERT_IDENTIFIER_LITERAL(&kft_mem, context, "func1", function1_type, true, false);
    ASSERT_IDENTIFIER_LITERAL(&kft_mem, context, "func2", function2_type, true, false);
    ASSERT_IDENTIFIER_LITERAL(&kft_mem, context, "X", context->type_traits->underlying_enumeration_type, true, false);
    ASSERT_IDENTIFIER_LITERAL(&kft_mem, context, "Y", context->type_traits->underlying_enumeration_type, true, false);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_IDENTIFIER_LITERAL

#define ASSERT_LABEL_ADDRESS(_mem, _context, _label, _point)                                                           \
    do {                                                                                                               \
        struct kefir_ast_label_address *addr = kefir_ast_new_label_address((_mem), (_context)->symbols, (_label));     \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(addr)));                              \
        ASSERT(addr->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                                  \
        ASSERT(KEFIR_AST_TYPE_SAME(kefir_ast_type_lvalue_conversion(addr->base.properties.type),                       \
                                   (kefir_ast_type_pointer((_mem), (_context)->type_bundle, kefir_ast_type_void())))); \
        ASSERT(addr->base.properties.expression_props.constant_expression);                                            \
        ASSERT(!addr->base.properties.expression_props.lvalue);                                                        \
        ASSERT(addr->base.properties.expression_props.scoped_id->label.point == (_point));                             \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(addr));                                                        \
    } while (0)

DEFINE_CASE(ast_node_analysis_label_address, "AST node analysis - label address") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_flow_control_structure *flow_control_structure = NULL;
    const struct kefir_ast_scoped_identifier *scoped_id_A = NULL;
    const struct kefir_ast_scoped_identifier *scoped_id_B = NULL;
    const struct kefir_ast_scoped_identifier *scoped_id_C = NULL;

    ASSERT_OK(kefir_ast_flow_control_tree_top(context->flow_control_tree, &flow_control_structure));
    ASSERT_OK(context->reference_label(&kft_mem, context, "A", flow_control_structure, NULL, &scoped_id_A));
    ASSERT_OK(context->reference_label(&kft_mem, context, "B", flow_control_structure, NULL, &scoped_id_B));
    ASSERT_OK(context->reference_label(&kft_mem, context, "C", flow_control_structure, NULL, &scoped_id_C));

    ASSERT_LABEL_ADDRESS(&kft_mem, context, "A", scoped_id_A->label.point);
    ASSERT_LABEL_ADDRESS(&kft_mem, context, "B", scoped_id_B->label.point);
    ASSERT_LABEL_ADDRESS(&kft_mem, context, "C", scoped_id_C->label.point);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_LABEL_ADDRESS

#define ASSERT_ARRAY_SUBSCRIPT(_mem, _context, _identifier, _index, _type, _const)                                     \
    do {                                                                                                       \
        struct kefir_ast_array_subscript *subscript = kefir_ast_new_array_subscript(                           \
            (_mem), KEFIR_AST_NODE_BASE(kefir_ast_new_identifier((_mem), (_context)->symbols, (_identifier))), \
            KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int((_mem), (_index))));                                \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(subscript)));                 \
        ASSERT(subscript->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                     \
        ASSERT(KEFIR_AST_TYPE_SAME(subscript->base.properties.type, (_type)));                                 \
        ASSERT(subscript->base.properties.expression_props.constant_expression == (_const));                              \
        ASSERT(subscript->base.properties.expression_props.lvalue);                                            \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(subscript));                                           \
                                                                                                               \
        struct kefir_ast_array_subscript *subscript2 = kefir_ast_new_array_subscript(                          \
            (_mem), KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int((_mem), (_index))),                         \
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier((_mem), (_context)->symbols, (_identifier))));        \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(subscript2)));                \
        ASSERT(subscript2->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                    \
        ASSERT(KEFIR_AST_TYPE_SAME(subscript2->base.properties.type, (_type)));                                \
        ASSERT(subscript2->base.properties.expression_props.constant_expression == (_const));                             \
        ASSERT(subscript2->base.properties.expression_props.lvalue);                                           \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(subscript2));                                          \
    } while (0)

#define ASSERT_ARRAY_SUBSCRIPT3(_mem, _context, _identifier, _index, _type, _cnst_expr)                                     \
    do {                                                                                                       \
        struct kefir_ast_array_subscript *subscript = kefir_ast_new_array_subscript(                           \
            (_mem), KEFIR_AST_NODE_BASE(kefir_ast_new_identifier((_mem), (_context)->symbols, (_identifier))), \
            KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int((_mem), (_index))));                                \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(subscript)));                 \
        ASSERT(subscript->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                     \
        ASSERT(KEFIR_AST_TYPE_SAME(subscript->base.properties.type, (_type)));                                 \
        ASSERT(subscript->base.properties.expression_props.constant_expression == (_cnst_expr));                              \
        ASSERT(subscript->base.properties.expression_props.lvalue);                                            \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(subscript));                                           \
                                                                                                               \
        struct kefir_ast_array_subscript *subscript2 = kefir_ast_new_array_subscript(                          \
            (_mem), KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int((_mem), (_index))),                         \
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier((_mem), (_context)->symbols, (_identifier))));        \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(subscript2)));                \
        ASSERT(subscript2->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                    \
        ASSERT(KEFIR_AST_TYPE_SAME(subscript2->base.properties.type, (_type)));                                \
        ASSERT(subscript2->base.properties.expression_props.constant_expression == (_cnst_expr));                             \
        ASSERT(subscript2->base.properties.expression_props.lvalue);                                           \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(subscript2));                                          \
    } while (0)

#define ASSERT_ARRAY_SUBSCRIPT2(_mem, _context, _identifier, _index1, _index2, _type, _const)                              \
    do {                                                                                                           \
        struct kefir_ast_array_subscript *subscript = kefir_ast_new_array_subscript(                               \
            (_mem),                                                                                                \
            KEFIR_AST_NODE_BASE(kefir_ast_new_array_subscript(                                                     \
                (_mem), KEFIR_AST_NODE_BASE(kefir_ast_new_identifier((_mem), (_context)->symbols, (_identifier))), \
                KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int((_mem), (_index1))))),                              \
            KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int((_mem), (_index2))));                                   \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(subscript)));                     \
        ASSERT(subscript->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                         \
        ASSERT(KEFIR_AST_TYPE_SAME(subscript->base.properties.type, (_type)));                                     \
        ASSERT(subscript->base.properties.expression_props.constant_expression == (_const));                                  \
        ASSERT(subscript->base.properties.expression_props.lvalue);                                                \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(subscript));                                               \
    } while (0)

DEFINE_CASE(ast_node_analysis_array_subscripts, "AST node analysis - array subscripts") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    const struct kefir_ast_type *array1 = kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_char());
    const struct kefir_ast_type *array2 =
        kefir_ast_type_unbounded_array(&kft_mem, context->type_bundle, kefir_ast_type_signed_short(), NULL);
    const struct kefir_ast_type *array3 =
        kefir_ast_type_array(&kft_mem, context->type_bundle, kefir_ast_type_signed_int(),
                             kefir_ast_constant_expression_integer(&kft_mem, 256), NULL);
    const struct kefir_ast_type *array4 =
        kefir_ast_type_array(&kft_mem, context->type_bundle,
                             kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_signed_long()),
                             kefir_ast_constant_expression_integer(&kft_mem, 10), NULL);
    const struct kefir_ast_type *array5 = kefir_ast_type_array(
        &kft_mem, context->type_bundle,
        kefir_ast_type_unbounded_array(&kft_mem, context->type_bundle, kefir_ast_type_signed_long_long(), NULL),
        kefir_ast_constant_expression_integer(&kft_mem, 10), NULL);
    const struct kefir_ast_type *array6 =
        kefir_ast_type_array(&kft_mem, context->type_bundle,
                             kefir_ast_type_array(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_long(),
                                                  kefir_ast_constant_expression_integer(&kft_mem, 12), NULL),
                             kefir_ast_constant_expression_integer(&kft_mem, 12), NULL);

    ASSERT_OK(
        kefir_ast_global_context_declare_external(&kft_mem, &global_context, "var1", array1, NULL, NULL, NULL, NULL));
    ASSERT_OK(
        kefir_ast_global_context_declare_external(&kft_mem, &global_context, "var2", array2, NULL, NULL, NULL, NULL));
    ASSERT_OK(
        kefir_ast_global_context_declare_external(&kft_mem, &global_context, "var3", array3, NULL, NULL, NULL, NULL));
    ASSERT_OK(
        kefir_ast_global_context_declare_external(&kft_mem, &global_context, "var4", array4, NULL, NULL, NULL, NULL));
    ASSERT_OK(
        kefir_ast_global_context_declare_external(&kft_mem, &global_context, "var5", array5, NULL, NULL, NULL, NULL));
    ASSERT_OK(
        kefir_ast_global_context_declare_external(&kft_mem, &global_context, "var6", array6, NULL, NULL, NULL, NULL));

    for (kefir_size_t i = 0; i < 10; i++) {
        ASSERT_ARRAY_SUBSCRIPT(&kft_mem, context, "var1", i, kefir_ast_type_char(), true);
        ASSERT_ARRAY_SUBSCRIPT(&kft_mem, context, "var2", i, kefir_ast_type_signed_short(), true);
        ASSERT_ARRAY_SUBSCRIPT(&kft_mem, context, "var3", i, kefir_ast_type_signed_int(), true);
        ASSERT_ARRAY_SUBSCRIPT(&kft_mem, context, "var4", i,
                               kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_signed_long()), true);
        ASSERT_ARRAY_SUBSCRIPT3(
            &kft_mem, context, "var5", i,
            kefir_ast_type_unbounded_array(&kft_mem, context->type_bundle, kefir_ast_type_signed_long_long(), NULL), true);
        ASSERT_ARRAY_SUBSCRIPT3(&kft_mem, context, "var6", i,
                               kefir_ast_type_array(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_long(),
                                                    kefir_ast_constant_expression_integer(&kft_mem, 12), NULL), true);

        ASSERT_ARRAY_SUBSCRIPT2(&kft_mem, context, "var4", i, i + 100, kefir_ast_type_signed_long(), true);
        ASSERT_ARRAY_SUBSCRIPT2(&kft_mem, context, "var5", i, i + 100, kefir_ast_type_signed_long_long(), true);
        ASSERT_ARRAY_SUBSCRIPT2(&kft_mem, context, "var6", i, i + 100, kefir_ast_type_unsigned_long(), true);
    }

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_ARRAY_SUBSCRIPT
#undef ASSERT_ARRAY_SUBSCRIPT2

#define ASSERT_STRUCT_MEMBER(_mem, _context, _identifier, _field, _type, _constant)                               \
    do {                                                                                                          \
        struct kefir_ast_struct_member *member = kefir_ast_new_struct_member(                                     \
            (_mem), (_context)->symbols,                                                                          \
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier((_mem), (_context)->symbols, (_identifier))), (_field)); \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(member)));                       \
        ASSERT(member->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                           \
        ASSERT(KEFIR_AST_TYPE_SAME(member->base.properties.type, (_type)));                                       \
        ASSERT(member->base.properties.expression_props.constant_expression == (_constant));                      \
        ASSERT(member->base.properties.expression_props.lvalue);                                                  \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(member));                                                 \
    } while (0)

DEFINE_CASE(ast_node_analysis_struct_members1, "AST node analysis - struct members #1") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_struct_type *struct1 = NULL;
    const struct kefir_ast_type *struct1_type =
        kefir_ast_type_structure(&kft_mem, context->type_bundle, "type1", &struct1);
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct1, "field1", kefir_ast_type_unsigned_char(),
                                          NULL));
    ASSERT_OK(kefir_ast_struct_type_field(
        &kft_mem, context->symbols, struct1, "field2",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_long()), NULL));
    ASSERT_OK(kefir_ast_struct_type_bitfield(&kft_mem, context->symbols, struct1, "field3", kefir_ast_type_signed_int(),
                                             NULL, kefir_ast_constant_expression_integer(&kft_mem, 3)));
    ASSERT_OK(kefir_ast_struct_type_field(
        &kft_mem, context->symbols, struct1, "field4",
        kefir_ast_type_unbounded_array(&kft_mem, context->type_bundle, kefir_ast_type_float(), NULL), NULL));

    ASSERT_OK(kefir_ast_global_context_define_external(&kft_mem, &global_context, "var1", struct1_type, NULL, NULL,
                                                       NULL, NULL, NULL));

    ASSERT_STRUCT_MEMBER(&kft_mem, context, "var1", "field1", kefir_ast_type_unsigned_char(), false);
    ASSERT_STRUCT_MEMBER(&kft_mem, context, "var1", "field2",
                         kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_long()), false);
    ASSERT_STRUCT_MEMBER(&kft_mem, context, "var1", "field3", kefir_ast_type_signed_int(), false);
    ASSERT_STRUCT_MEMBER(&kft_mem, context, "var1", "field4",
                         kefir_ast_type_unbounded_array(&kft_mem, context->type_bundle, kefir_ast_type_float(), NULL),
                         true);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_struct_members2, "AST node analysis - struct members #2") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_struct_type *struct2 = NULL;
    const struct kefir_ast_type *struct2_type =
        kefir_ast_type_structure(&kft_mem, context->type_bundle, "type2", &struct2);
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct2, "field1", kefir_ast_type_float(), NULL));
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct2, "field2", kefir_ast_type_unsigned_int(),
                                          NULL));

    struct kefir_ast_struct_type *struct3 = NULL;
    const struct kefir_ast_type *struct3_type =
        kefir_ast_type_structure(&kft_mem, context->type_bundle, "type3", &struct3);
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct3, NULL, struct2_type, NULL));
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct3, "test", kefir_ast_type_boolean(), NULL));
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct3, "test2", struct2_type, NULL));

    ASSERT_OK(kefir_ast_global_context_define_external(&kft_mem, &global_context, "var2", struct3_type, NULL, NULL,
                                                       NULL, NULL, NULL));
    ASSERT_STRUCT_MEMBER(&kft_mem, context, "var2", "test", kefir_ast_type_boolean(), false);
    ASSERT_STRUCT_MEMBER(&kft_mem, context, "var2", "test2", struct2_type, false);
    ASSERT_STRUCT_MEMBER(&kft_mem, context, "var2", "field1", kefir_ast_type_float(), false);
    ASSERT_STRUCT_MEMBER(&kft_mem, context, "var2", "field2", kefir_ast_type_unsigned_int(), false);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_STRUCT_MEMBER

#define ASSERT_INDIRECT_STRUCT_MEMBER(_mem, _context, _identifier, _field, _type, _constant)                      \
    do {                                                                                                          \
        struct kefir_ast_struct_member *member = kefir_ast_new_struct_indirect_member(                            \
            (_mem), (_context)->symbols,                                                                          \
            KEFIR_AST_NODE_BASE(kefir_ast_new_identifier((_mem), (_context)->symbols, (_identifier))), (_field)); \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(member)));                       \
        ASSERT(member->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                           \
        ASSERT(KEFIR_AST_TYPE_SAME(member->base.properties.type, (_type)));                                       \
        ASSERT(member->base.properties.expression_props.constant_expression == (_constant));                      \
        ASSERT(member->base.properties.expression_props.lvalue);                                                  \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(member));                                                 \
    } while (0)

DEFINE_CASE(ast_node_analysis_indirect_struct_members1, "AST node analysis - indirect struct members #1") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_struct_type *struct1 = NULL;
    const struct kefir_ast_type *struct1_type =
        kefir_ast_type_structure(&kft_mem, context->type_bundle, "type1", &struct1);
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct1, "field1", kefir_ast_type_unsigned_char(),
                                          NULL));
    ASSERT_OK(kefir_ast_struct_type_field(
        &kft_mem, context->symbols, struct1, "field2",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_long()), NULL));
    ASSERT_OK(kefir_ast_struct_type_bitfield(&kft_mem, context->symbols, struct1, "field3", kefir_ast_type_signed_int(),
                                             NULL, kefir_ast_constant_expression_integer(&kft_mem, 3)));
    ASSERT_OK(kefir_ast_struct_type_field(
        &kft_mem, context->symbols, struct1, "field4",
        kefir_ast_type_unbounded_array(&kft_mem, context->type_bundle, kefir_ast_type_float(), NULL), NULL));

    ASSERT_OK(kefir_ast_global_context_define_external(
        &kft_mem, &global_context, "var1", kefir_ast_type_pointer(&kft_mem, context->type_bundle, struct1_type), NULL,
        NULL, NULL, NULL, NULL));

    ASSERT_INDIRECT_STRUCT_MEMBER(&kft_mem, context, "var1", "field1", kefir_ast_type_unsigned_char(), false);
    ASSERT_INDIRECT_STRUCT_MEMBER(
        &kft_mem, context, "var1", "field2",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_unsigned_long()), false);
    ASSERT_INDIRECT_STRUCT_MEMBER(&kft_mem, context, "var1", "field3", kefir_ast_type_signed_int(), false);
    ASSERT_INDIRECT_STRUCT_MEMBER(
        &kft_mem, context, "var1", "field4",
        kefir_ast_type_unbounded_array(&kft_mem, context->type_bundle, kefir_ast_type_float(), NULL), false);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_indirect_struct_members2, "AST node analysis - indirect struct members #2") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_struct_type *struct2 = NULL;
    const struct kefir_ast_type *struct2_type =
        kefir_ast_type_structure(&kft_mem, context->type_bundle, "type2", &struct2);
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct2, "field1", kefir_ast_type_float(), NULL));
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct2, "field2", kefir_ast_type_unsigned_int(),
                                          NULL));

    struct kefir_ast_struct_type *struct3 = NULL;
    const struct kefir_ast_type *struct3_type =
        kefir_ast_type_structure(&kft_mem, context->type_bundle, "type3", &struct3);
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct3, NULL, struct2_type, NULL));
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct3, "test", kefir_ast_type_boolean(), NULL));
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, context->symbols, struct3, "test2", struct2_type, NULL));

    ASSERT_OK(kefir_ast_global_context_define_external(
        &kft_mem, &global_context, "var2", kefir_ast_type_pointer(&kft_mem, context->type_bundle, struct3_type), NULL,
        NULL, NULL, NULL, NULL));
    ASSERT_INDIRECT_STRUCT_MEMBER(&kft_mem, context, "var2", "test", kefir_ast_type_boolean(), false);
    ASSERT_INDIRECT_STRUCT_MEMBER(&kft_mem, context, "var2", "test2", struct2_type, false);
    ASSERT_INDIRECT_STRUCT_MEMBER(&kft_mem, context, "var2", "field1", kefir_ast_type_float(), false);
    ASSERT_INDIRECT_STRUCT_MEMBER(&kft_mem, context, "var2", "field2", kefir_ast_type_unsigned_int(), false);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_INDIRECT_STRUCT_MEMBER

#define ASSERT_FUNCTION_CALL(_mem, _context, _id, _type)                                                       \
    do {                                                                                                       \
        struct kefir_ast_function_call *call1 = kefir_ast_new_function_call(                                   \
            (_mem), KEFIR_AST_NODE_BASE(kefir_ast_new_identifier((_mem), (_context)->symbols, (_id))));        \
        ASSERT_OK(kefir_ast_function_call_append((_mem), call1,                                                \
                                                 KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int((_mem), 0)))); \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(call1)));                     \
        ASSERT(call1->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);                         \
        ASSERT(KEFIR_AST_TYPE_SAME(call1->base.properties.type, (_type)));                                     \
        ASSERT(!call1->base.properties.expression_props.constant_expression);                                  \
        ASSERT(!call1->base.properties.expression_props.lvalue);                                               \
        ASSERT(!call1->base.properties.expression_props.addressable);                                          \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(call1));                                               \
    } while (0)

#define ASSERT_FUNCTION_CALL_NOK(_mem, _context, _id)                                                          \
    do {                                                                                                       \
        struct kefir_ast_function_call *call1 = kefir_ast_new_function_call(                                   \
            (_mem), KEFIR_AST_NODE_BASE(kefir_ast_new_identifier((_mem), (_context)->symbols, (_id))));        \
        ASSERT_OK(kefir_ast_function_call_append((_mem), call1,                                                \
                                                 KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int((_mem), 0)))); \
        ASSERT_NOK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(call1)));                    \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(call1));                                               \
    } while (0)

DEFINE_CASE(ast_node_analysis_function_calls, "AST node analysis - function calls") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_function_type *function1 = NULL;
    const struct kefir_ast_type *function1_type =
        kefir_ast_type_function(&kft_mem, context->type_bundle, kefir_ast_type_signed_long_long(), &function1);
    ASSERT_OK(kefir_ast_type_function_parameter(&kft_mem, context->type_bundle, function1,
                                                kefir_ast_type_unsigned_int(), NULL));

    struct kefir_ast_function_type *function2 = NULL;
    const struct kefir_ast_type *function2_type =
        kefir_ast_type_function(&kft_mem, context->type_bundle, kefir_ast_type_signed_long_long(), &function2);
    ASSERT_OK(
        kefir_ast_type_function_parameter(&kft_mem, context->type_bundle, function2, kefir_ast_type_float(), NULL));
    ASSERT_OK(
        kefir_ast_type_function_parameter(&kft_mem, context->type_bundle, function2, kefir_ast_type_float(), NULL));

    const struct kefir_ast_type *function1_ptr_type =
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, function1_type);

    ASSERT_OK(kefir_ast_local_context_declare_function(&kft_mem, &local_context, KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                                                       true, "func1", function1_type, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_function(&kft_mem, &local_context, KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                                                       true, "func3", function2_type, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_define_auto(&kft_mem, &local_context, "func2", function1_ptr_type, NULL, NULL,
                                                  NULL, NULL, NULL));

    ASSERT_FUNCTION_CALL(&kft_mem, context, "func1", kefir_ast_type_signed_long_long());
    ASSERT_FUNCTION_CALL(&kft_mem, context, "func2", kefir_ast_type_signed_long_long());
    ASSERT_FUNCTION_CALL_NOK(&kft_mem, context, "func3");

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_function_call_qualified_rvalues,
            "AST node analysis - function call returning qualified rvalues") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    struct kefir_ast_function_type *function1 = NULL;
    const struct kefir_ast_type *function1_type =
        kefir_ast_type_function(&kft_mem, context->type_bundle, kefir_ast_type_signed_long_long(), &function1);
    ASSERT_OK(kefir_ast_type_function_parameter(&kft_mem, context->type_bundle, function1,
                                                kefir_ast_type_unsigned_int(), NULL));

    ASSERT_OK(kefir_ast_local_context_declare_function(&kft_mem, &local_context, KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                                                       true, "func1", function1_type, NULL, NULL, NULL));

    struct kefir_ast_function_type *function2 = NULL;
    const struct kefir_ast_type *function2_type = kefir_ast_type_function(
        &kft_mem, context->type_bundle,
        kefir_ast_type_qualified(&kft_mem, local_context.context.type_bundle, kefir_ast_type_signed_long_long(),
                                 (struct kefir_ast_type_qualification){.constant = true}),
        &function2);
    ASSERT_OK(kefir_ast_type_function_parameter(&kft_mem, context->type_bundle, function2,
                                                kefir_ast_type_unsigned_int(), NULL));

    struct kefir_ast_function_type *function3 = NULL;
    const struct kefir_ast_type *function3_type = kefir_ast_type_function(
        &kft_mem, context->type_bundle,
        kefir_ast_type_qualified(&kft_mem, local_context.context.type_bundle, kefir_ast_type_signed_long_long(),
                                 (struct kefir_ast_type_qualification){.constant = true, .restricted = true}),
        &function3);
    ASSERT_OK(kefir_ast_type_function_parameter(&kft_mem, context->type_bundle, function3,
                                                kefir_ast_type_unsigned_int(), NULL));

    const struct kefir_ast_type *fn4_type =
        kefir_ast_type_qualified(&kft_mem, local_context.context.type_bundle, kefir_ast_type_signed_long_long(),
                                 (struct kefir_ast_type_qualification){.constant = true, .restricted = true});
    struct kefir_ast_function_type *function4 = NULL;
    const struct kefir_ast_type *function4_type = kefir_ast_type_function(
        &kft_mem, context->type_bundle,
        kefir_ast_type_qualified(&kft_mem, local_context.context.type_bundle,
                                 kefir_ast_type_pointer(&kft_mem, local_context.context.type_bundle, fn4_type),
                                 (struct kefir_ast_type_qualification){.restricted = true}),
        &function4);
    ASSERT_OK(kefir_ast_type_function_parameter(&kft_mem, context->type_bundle, function4,
                                                kefir_ast_type_unsigned_int(), NULL));

    ASSERT_OK(kefir_ast_local_context_declare_function(&kft_mem, &local_context, KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                                                       true, "func1", function1_type, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_function(&kft_mem, &local_context, KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                                                       true, "func2", function2_type, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_function(&kft_mem, &local_context, KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                                                       true, "func3", function3_type, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_function(&kft_mem, &local_context, KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                                                       true, "func4", function4_type, NULL, NULL, NULL));

    ASSERT_FUNCTION_CALL(&kft_mem, context, "func1", kefir_ast_type_signed_long_long());
    ASSERT_FUNCTION_CALL(&kft_mem, context, "func2", kefir_ast_type_signed_long_long());
    ASSERT_FUNCTION_CALL(&kft_mem, context, "func3", kefir_ast_type_signed_long_long());
    ASSERT_FUNCTION_CALL(&kft_mem, context, "func4",
                         kefir_ast_type_pointer(&kft_mem, local_context.context.type_bundle, fn4_type));

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_FUNCTION_CALL

#define ASSERT_UNARY_OPERATION(_mem, _context, _oper, _arg, _type, _constant, _lvalue, _addresable) \
    do {                                                                                            \
        struct kefir_ast_unary_operation *oper =                                                    \
            kefir_ast_new_unary_operation((_mem), (_oper), KEFIR_AST_NODE_BASE((_arg)));            \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), KEFIR_AST_NODE_BASE(oper)));           \
        ASSERT(oper->base.properties.category == KEFIR_AST_NODE_CATEGORY_EXPRESSION);               \
        ASSERT(KEFIR_AST_TYPE_SAME(oper->base.properties.type, (_type)));                           \
        ASSERT(oper->base.properties.expression_props.constant_expression == (_constant));          \
        ASSERT(oper->base.properties.expression_props.lvalue == (_lvalue));                         \
        ASSERT(oper->base.properties.expression_props.addressable == (_addresable));                \
        ASSERT(!oper->base.properties.expression_props.bitfield_props.bitfield);                    \
        KEFIR_AST_NODE_FREE((_mem), KEFIR_AST_NODE_BASE(oper));                                     \
    } while (0)

#define ASSERT_UNARY_ARITH_OPERATION(_mem, _context, _oper)                                                  \
    do {                                                                                                     \
        ASSERT_UNARY_OPERATION((_mem), (_context), (_oper), kefir_ast_new_constant_bool((_mem), false),      \
                               kefir_ast_type_signed_int(), true, false, false);                             \
        ASSERT_UNARY_OPERATION((_mem), (_context), (_oper), kefir_ast_new_constant_char((_mem), 'a'),        \
                               kefir_ast_type_signed_int(), true, false, false);                             \
        ASSERT_UNARY_OPERATION((_mem), (_context), (_oper), kefir_ast_new_constant_int((_mem), -100),        \
                               kefir_ast_type_signed_int(), true, false, false);                             \
        ASSERT_UNARY_OPERATION((_mem), (_context), (_oper), kefir_ast_new_constant_uint((_mem), 100),        \
                               kefir_ast_type_unsigned_int(), true, false, false);                           \
        ASSERT_UNARY_OPERATION((_mem), (_context), (_oper), kefir_ast_new_constant_long((_mem), -1000),      \
                               kefir_ast_type_signed_long(), true, false, false);                            \
        ASSERT_UNARY_OPERATION((_mem), (_context), (_oper), kefir_ast_new_constant_ulong((_mem), 1000),      \
                               kefir_ast_type_unsigned_long(), true, false, false);                          \
        ASSERT_UNARY_OPERATION((_mem), (_context), (_oper), kefir_ast_new_constant_long_long((_mem), -1000), \
                               kefir_ast_type_signed_long_long(), true, false, false);                       \
        ASSERT_UNARY_OPERATION((_mem), (_context), (_oper), kefir_ast_new_constant_ulong_long((_mem), 1000), \
                               kefir_ast_type_unsigned_long_long(), true, false, false);                     \
    } while (0)

DEFINE_CASE(ast_node_analysis_unary_operation_arithmetic, "AST node analysis - unary arithmetic operations") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_UNARY_ARITH_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PLUS);
    ASSERT_UNARY_ARITH_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_NEGATE);
    ASSERT_UNARY_ARITH_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_INVERT);

    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PLUS, kefir_ast_new_constant_float(&kft_mem, 3.14f),
                           kefir_ast_type_float(), true, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PLUS,
                           kefir_ast_new_constant_double(&kft_mem, 2.71828), kefir_ast_type_double(), true, false,
                           false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PLUS,
                           kefir_ast_new_constant_long_double(&kft_mem, -1.298l), kefir_ast_type_long_double(), true,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_NEGATE, kefir_ast_new_constant_float(&kft_mem, 3.14f),
                           kefir_ast_type_float(), true, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_NEGATE,
                           kefir_ast_new_constant_double(&kft_mem, 2.71828), kefir_ast_type_double(), true, false,
                           false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_NEGATE,
                           kefir_ast_new_constant_long_double(&kft_mem, -0.286l), kefir_ast_type_long_double(), true,
                           false, false);

    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_NEGATE,
                           kefir_ast_new_constant_bool(&kft_mem, false), kefir_ast_type_signed_int(), true, false,
                           false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_NEGATE,
                           kefir_ast_new_constant_char(&kft_mem, 'a'), kefir_ast_type_signed_int(), true, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_NEGATE,
                           kefir_ast_new_constant_int(&kft_mem, -100), kefir_ast_type_signed_int(), true, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_NEGATE,
                           kefir_ast_new_constant_uint(&kft_mem, 100), kefir_ast_type_signed_int(), true, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_NEGATE,
                           kefir_ast_new_constant_long(&kft_mem, -1000), kefir_ast_type_signed_int(), true, false,
                           false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_NEGATE,
                           kefir_ast_new_constant_ulong(&kft_mem, 1000), kefir_ast_type_signed_int(), true, false,
                           false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_NEGATE,
                           kefir_ast_new_constant_long_long(&kft_mem, -1000), kefir_ast_type_signed_int(), true, false,
                           false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_LOGICAL_NEGATE,
                           kefir_ast_new_constant_ulong_long(&kft_mem, 1000), kefir_ast_type_signed_int(), true, false,
                           false);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_UNARY_ARITH_OPERATION

DEFINE_CASE(ast_node_analysis_unary_operation_address, "AST node analysis - unary address operations") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    const struct kefir_ast_type *type1 = kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_char());
    const struct kefir_ast_type *type2 =
        kefir_ast_type_array(&kft_mem, context->type_bundle, kefir_ast_type_float(),
                             kefir_ast_constant_expression_integer(&kft_mem, 100), NULL);

    struct kefir_ast_function_type *function3 = NULL;
    const struct kefir_ast_type *function_type3 =
        kefir_ast_type_function(&kft_mem, context->type_bundle, kefir_ast_type_void(), &function3);

    ASSERT_OK(kefir_ast_global_context_declare_external(
        &kft_mem, &global_context, "var0",
        kefir_ast_type_qualified(&kft_mem, context->type_bundle, kefir_ast_type_signed_long(),
                                 (struct kefir_ast_type_qualification){.constant = true}),
        NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "var1", kefir_ast_type_signed_int(),
                                                       NULL, NULL, NULL, NULL));
    ASSERT_OK(
        kefir_ast_local_context_define_static(&kft_mem, &local_context, "var2", type1, NULL, NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_define_static_thread_local(&kft_mem, &local_context, "var3", type2, NULL, NULL,
                                                                 NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_function(&kft_mem, &local_context, KEFIR_AST_FUNCTION_SPECIFIER_NONE,
                                                       true, "func1", function_type3, NULL, NULL, NULL));

    ASSERT_UNARY_OPERATION(
        &kft_mem, context, KEFIR_AST_OPERATION_ADDRESS, kefir_ast_new_identifier(&kft_mem, context->symbols, "var0"),
        kefir_ast_type_pointer(&kft_mem, context->type_bundle,
                               kefir_ast_type_qualified(&kft_mem, context->type_bundle, kefir_ast_type_signed_long(),
                                                        (struct kefir_ast_type_qualification){.constant = true})),
        true, false, false);
    ASSERT_UNARY_OPERATION(
        &kft_mem, context, KEFIR_AST_OPERATION_ADDRESS, kefir_ast_new_identifier(&kft_mem, context->symbols, "var1"),
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_signed_int()), true, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_ADDRESS,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var2"),
                           kefir_ast_type_pointer(&kft_mem, context->type_bundle, type1), true, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_ADDRESS,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var3"),
                           kefir_ast_type_pointer(&kft_mem, context->type_bundle, type2), false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_ADDRESS,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "func1"),
                           kefir_ast_type_pointer(&kft_mem, context->type_bundle, function_type3), true, false, false);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_unary_operation_indirect, "AST node analysis - unary indirect operations") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;
    const struct kefir_ast_type *type1 = kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_char());
    const struct kefir_ast_type *type2 =
        kefir_ast_type_array(&kft_mem, context->type_bundle, kefir_ast_type_float(),
                             kefir_ast_constant_expression_integer(&kft_mem, 100), NULL);

    struct kefir_ast_function_type *function3 = NULL;
    const struct kefir_ast_type *function_type3 =
        kefir_ast_type_function(&kft_mem, context->type_bundle, kefir_ast_type_double(), &function3);
    ASSERT_OK(
        kefir_ast_type_function_parameter(&kft_mem, context->type_bundle, function3, kefir_ast_type_boolean(), NULL));

    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "var1",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_signed_int()), NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "var2",
                                                       kefir_ast_type_pointer(&kft_mem, context->type_bundle, type1),
                                                       NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "var3",
                                                       kefir_ast_type_pointer(&kft_mem, context->type_bundle, type2),
                                                       NULL, NULL, NULL, NULL));
    ASSERT_OK(
        kefir_ast_local_context_declare_external(&kft_mem, &local_context, "var4", type2, NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "var5", kefir_ast_type_pointer(&kft_mem, context->type_bundle, function_type3), NULL,
        NULL, NULL, NULL));

    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_INDIRECTION,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var1"), kefir_ast_type_signed_int(),
                           false, true, true);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_INDIRECTION,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var2"), type1, false, true, true);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_INDIRECTION,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var3"), type2, false, true, true);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_INDIRECTION,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var4"), kefir_ast_type_float(), false,
                           true, true);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_INDIRECTION,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var5"), function_type3, false, false,
                           true);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_unary_operation_incdec, "AST node analysis - unary increment/decrement operations") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "var1", kefir_ast_type_boolean(), NULL,
                                                       NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "var2", kefir_ast_type_signed_int(),
                                                       NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "var3",
                                                       kefir_ast_type_unsigned_long_long(), NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "var4", kefir_ast_type_float(), NULL,
                                                       NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "var5",
        kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_float()), NULL, NULL, NULL, NULL));

    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_POSTFIX_DECREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var1"), kefir_ast_type_boolean(),
                           false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_POSTFIX_DECREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var2"), kefir_ast_type_signed_int(),
                           false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_POSTFIX_DECREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var3"),
                           kefir_ast_type_unsigned_long_long(), false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_POSTFIX_DECREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var4"), kefir_ast_type_float(), false,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_POSTFIX_DECREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var5"),
                           kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_float()), false, false,
                           false);

    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_POSTFIX_INCREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var1"), kefir_ast_type_boolean(),
                           false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_POSTFIX_INCREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var2"), kefir_ast_type_signed_int(),
                           false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_POSTFIX_INCREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var3"),
                           kefir_ast_type_unsigned_long_long(), false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_POSTFIX_INCREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var4"), kefir_ast_type_float(), false,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_POSTFIX_INCREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var5"),
                           kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_float()), false, false,
                           false);

    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PREFIX_DECREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var1"), kefir_ast_type_boolean(),
                           false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PREFIX_DECREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var2"), kefir_ast_type_signed_int(),
                           false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PREFIX_DECREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var3"),
                           kefir_ast_type_unsigned_long_long(), false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PREFIX_DECREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var4"), kefir_ast_type_float(), false,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PREFIX_DECREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var5"),
                           kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_float()), false, false,
                           false);

    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PREFIX_INCREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var1"), kefir_ast_type_boolean(),
                           false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PREFIX_INCREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var2"), kefir_ast_type_signed_int(),
                           false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PREFIX_INCREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var3"),
                           kefir_ast_type_unsigned_long_long(), false, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PREFIX_INCREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var4"), kefir_ast_type_float(), false,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_PREFIX_INCREMENT,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "var5"),
                           kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_float()), false, false,
                           false);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_unary_operation_sizeof, "AST node analysis - unary sizeof operations") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

    ASSERT_OK(kefir_ast_local_context_declare_external(&kft_mem, &local_context, "x", kefir_ast_type_unsigned_int(),
                                                       NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_declare_external(
        &kft_mem, &local_context, "y",
        kefir_ast_type_array(&kft_mem, context->type_bundle, kefir_ast_type_char(),
                             kefir_ast_constant_expression_integer(&kft_mem, 1), NULL),
        NULL, NULL, NULL, NULL));
    ASSERT_OK(kefir_ast_local_context_define_auto(
        &kft_mem, &local_context, "z",
        kefir_ast_type_vlen_array(&kft_mem, context->type_bundle, kefir_ast_type_char(),
                                  KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 128)), NULL),
        NULL, NULL, NULL, NULL, NULL));

    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_SIZEOF, kefir_ast_new_constant_bool(&kft_mem, false),
                           type_traits->size_type, true, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_SIZEOF,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "x"), type_traits->size_type, true,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_SIZEOF,
                           KEFIR_AST_MAKE_STRING_LITERAL_MULTIBYTE(&kft_mem, "Hello, world!"), type_traits->size_type,
                           true, false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_SIZEOF,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "y"), type_traits->size_type, true,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_SIZEOF,
                           kefir_ast_new_identifier(&kft_mem, context->symbols, "z"), type_traits->size_type, false,
                           false, false);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

DEFINE_CASE(ast_node_analysis_unary_operation_alignof, "AST node analysis - unary alignof operations") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

#define MAKE_TYPENAME(_id, _spec_count, ...)                                                      \
    struct kefir_ast_type_name *_id =                                                             \
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL)); \
    ASSERT_OK(append_specifiers(&kft_mem, &_id->type_decl.specifiers, (_spec_count), __VA_ARGS__));

    MAKE_TYPENAME(type_name1, 1, kefir_ast_type_specifier_boolean(&kft_mem));
    MAKE_TYPENAME(type_name2, 2, kefir_ast_type_specifier_unsigned(&kft_mem), kefir_ast_type_specifier_short(&kft_mem));
    MAKE_TYPENAME(type_name3, 2, kefir_ast_type_specifier_signed(&kft_mem), kefir_ast_type_specifier_int(&kft_mem));
    MAKE_TYPENAME(type_name4, 3, kefir_ast_type_specifier_unsigned(&kft_mem), kefir_ast_type_specifier_long(&kft_mem),
                  kefir_ast_type_specifier_long(&kft_mem));
    MAKE_TYPENAME(type_name5, 1, kefir_ast_type_specifier_float(&kft_mem));
#undef MAKE_TYPENAME

    struct kefir_ast_type_name *type_name6 = kefir_ast_new_type_name(
        &kft_mem, kefir_ast_declarator_array(&kft_mem, KEFIR_AST_DECLARATOR_ARRAY_BOUNDED,
                                             KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 150)),
                                             kefir_ast_declarator_identifier(&kft_mem, NULL, NULL)));
    ASSERT_OK(
        append_specifiers(&kft_mem, &type_name6->type_decl.specifiers, 1, kefir_ast_type_specifier_char(&kft_mem)));
    type_name6->type_decl.declarator->array.static_array = true;

    struct kefir_ast_type_name *type_name7 = kefir_ast_new_type_name(
        &kft_mem, kefir_ast_declarator_pointer(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL)));
    ASSERT_OK(append_specifiers(&kft_mem, &type_name7->type_decl.specifiers, 2, kefir_ast_type_specifier_char(&kft_mem),
                                kefir_ast_type_qualifier_const(&kft_mem)));

    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_ALIGNOF, type_name1, type_traits->size_type, true,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_ALIGNOF, type_name2, type_traits->size_type, true,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_ALIGNOF, type_name3, type_traits->size_type, true,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_ALIGNOF, type_name4, type_traits->size_type, true,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_ALIGNOF, type_name5, type_traits->size_type, true,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_ALIGNOF, type_name6, type_traits->size_type, true,
                           false, false);
    ASSERT_UNARY_OPERATION(&kft_mem, context, KEFIR_AST_OPERATION_ALIGNOF, type_name7, type_traits->size_type, true,
                           false, false);

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

#undef ASSERT_UNARY_OPERATION

#define ASSERT_TYPE_NAME(_mem, _context, _type_name, _type)                      \
    do {                                                                         \
        struct kefir_ast_node_base *type_name = KEFIR_AST_NODE_BASE(_type_name); \
        ASSERT_OK(kefir_ast_analyze_node((_mem), (_context), type_name));        \
        ASSERT(type_name->properties.category == KEFIR_AST_NODE_CATEGORY_TYPE);  \
        ASSERT(KEFIR_AST_TYPE_SAME(type_name->properties.type, (_type)));        \
        ASSERT(!type_name->properties.expression_props.constant_expression);     \
        ASSERT(!type_name->properties.expression_props.lvalue);                  \
        ASSERT(!type_name->properties.expression_props.addressable);             \
        KEFIR_AST_NODE_FREE((_mem), type_name);                                  \
    } while (0)

DEFINE_CASE(ast_node_analysis_type_name, "AST node analysis - type names") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_global_context global_context;
    struct kefir_ast_local_context local_context;

    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, type_traits, &kft_util_get_translator_environment()->target_env,
                                            &global_context, NULL));
    ASSERT_OK(kefir_ast_local_context_init(&kft_mem, &global_context, &local_context));
    struct kefir_ast_context *context = &local_context.context;

#define MAKE_TYPENAME(_id, _spec_count, ...)                                                      \
    struct kefir_ast_type_name *_id =                                                             \
        kefir_ast_new_type_name(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL)); \
    ASSERT_OK(append_specifiers(&kft_mem, &_id->type_decl.specifiers, (_spec_count), __VA_ARGS__));

    MAKE_TYPENAME(type_name1, 1, kefir_ast_type_specifier_boolean(&kft_mem));
    MAKE_TYPENAME(type_name2, 2, kefir_ast_type_specifier_signed(&kft_mem), kefir_ast_type_specifier_char(&kft_mem));
    MAKE_TYPENAME(type_name3, 2, kefir_ast_type_specifier_signed(&kft_mem), kefir_ast_type_specifier_int(&kft_mem));
    MAKE_TYPENAME(type_name4, 3, kefir_ast_type_specifier_unsigned(&kft_mem), kefir_ast_type_specifier_long(&kft_mem),
                  kefir_ast_type_specifier_long(&kft_mem));
    MAKE_TYPENAME(type_name5, 1, kefir_ast_type_specifier_float(&kft_mem));
    MAKE_TYPENAME(type_name6, 1, kefir_ast_type_specifier_double(&kft_mem));
#undef MAKE_TYPENAME

    struct kefir_ast_type_name *type_name7 = kefir_ast_new_type_name(
        &kft_mem, kefir_ast_declarator_pointer(&kft_mem, kefir_ast_declarator_identifier(&kft_mem, NULL, NULL)));
    ASSERT_OK(
        append_specifiers(&kft_mem, &type_name7->type_decl.specifiers, 1, kefir_ast_type_specifier_void(&kft_mem)));

    struct kefir_ast_type_name *type_name8 = kefir_ast_new_type_name(
        &kft_mem,
        kefir_ast_declarator_pointer(
            &kft_mem, kefir_ast_declarator_array(&kft_mem, KEFIR_AST_DECLARATOR_ARRAY_BOUNDED,
                                                 KEFIR_AST_NODE_BASE(kefir_ast_new_constant_int(&kft_mem, 256)),
                                                 kefir_ast_declarator_identifier(&kft_mem, NULL, NULL))));
    ASSERT_OK(
        append_specifiers(&kft_mem, &type_name8->type_decl.specifiers, 1, kefir_ast_type_specifier_void(&kft_mem)));

    ASSERT_TYPE_NAME(&kft_mem, context, type_name1, kefir_ast_type_boolean());
    ASSERT_TYPE_NAME(&kft_mem, context, type_name2, kefir_ast_type_signed_char());
    ASSERT_TYPE_NAME(&kft_mem, context, type_name3, kefir_ast_type_signed_int());
    ASSERT_TYPE_NAME(&kft_mem, context, type_name4, kefir_ast_type_unsigned_long_long());
    ASSERT_TYPE_NAME(&kft_mem, context, type_name5, kefir_ast_type_float());
    ASSERT_TYPE_NAME(&kft_mem, context, type_name6, kefir_ast_type_double());
    ASSERT_TYPE_NAME(&kft_mem, context, type_name7,
                     kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_void()));
    ASSERT_TYPE_NAME(&kft_mem, context, type_name8,
                     kefir_ast_type_array(&kft_mem, context->type_bundle,
                                          kefir_ast_type_pointer(&kft_mem, context->type_bundle, kefir_ast_type_void()),
                                          kefir_ast_constant_expression_integer(&kft_mem, 256), NULL));

    ASSERT_OK(kefir_ast_local_context_free(&kft_mem, &local_context));
    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

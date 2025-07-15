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

#include "kefir/test/unit_test.h"
#include "kefir/test/util.h"
#include "kefir/ast/type.h"
#include "kefir/ast/analyzer/analyzer.h"

DEFINE_CASE(ast_nullptr_type1, "AST Type analysis - nullptr type #1") {
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();
    struct kefir_ast_type_bundle type_bundle;
    struct kefir_string_pool symbols;
    ASSERT_OK(kefir_string_pool_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));

    const struct kefir_ast_type *nullptr_type = kefir_ast_type_nullptr();
    ASSERT(nullptr_type != NULL);
    ASSERT(nullptr_type == kefir_ast_type_nullptr());
    ASSERT(nullptr_type->tag == KEFIR_AST_TYPE_SCALAR_NULL_POINTER);
    ASSERT(KEFIR_AST_TYPE_SAME(nullptr_type, nullptr_type));
    ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, nullptr_type, nullptr_type));

    const struct kefir_ast_type *nullptr_composite =
        KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, nullptr_type, nullptr_type);
    ASSERT(nullptr_composite == nullptr_type);

    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_nullptr_assignable1, "AST Type analysis - nullptr type assignable #1") {
    struct kefir_ast_translator_environment env;
    ASSERT_OK(kefir_ast_translator_environment_init(&env, kft_util_get_ir_target_platform()));

    struct kefir_ast_global_context global_context;
    ASSERT_OK(kefir_ast_global_context_init(&kft_mem, kefir_util_default_type_traits(), &env.target_env,
                                            &global_context, NULL));

    const struct kefir_ast_type *nullptr_type = kefir_ast_type_nullptr();
    ASSERT_OK(kefir_ast_type_assignable(&kft_mem, &global_context.context, nullptr_type, true, nullptr_type));
    ASSERT_OK(kefir_ast_type_assignable(&kft_mem, &global_context.context, nullptr_type, false, nullptr_type));
    ASSERT_OK(
        kefir_ast_type_assignable(&kft_mem, &global_context.context, nullptr_type, true, kefir_ast_type_boolean()));
    ASSERT_OK(
        kefir_ast_type_assignable(&kft_mem, &global_context.context, nullptr_type, false, kefir_ast_type_boolean()));
    ASSERT_OK(kefir_ast_type_assignable(
        &kft_mem, &global_context.context, nullptr_type, true,
        kefir_ast_type_pointer(&kft_mem, global_context.context.type_bundle, kefir_ast_type_void())));
    ASSERT_OK(kefir_ast_type_assignable(
        &kft_mem, &global_context.context, nullptr_type, false,
        kefir_ast_type_pointer(&kft_mem, global_context.context.type_bundle, kefir_ast_type_void())));
    ASSERT_OK(kefir_ast_type_assignable(
        &kft_mem, &global_context.context, nullptr_type, true,
        kefir_ast_type_pointer(&kft_mem, global_context.context.type_bundle, kefir_ast_type_signed_int())));
    ASSERT_OK(kefir_ast_type_assignable(
        &kft_mem, &global_context.context, nullptr_type, false,
        kefir_ast_type_pointer(&kft_mem, global_context.context.type_bundle, kefir_ast_type_signed_int())));

    ASSERT_NOK(
        kefir_ast_type_assignable(&kft_mem, &global_context.context, kefir_ast_type_signed_int(), true, nullptr_type));
    ASSERT_NOK(
        kefir_ast_type_assignable(&kft_mem, &global_context.context, kefir_ast_type_signed_int(), false, nullptr_type));
    ASSERT_NOK(kefir_ast_type_assignable(
        &kft_mem, &global_context.context,
        kefir_ast_type_pointer(&kft_mem, global_context.context.type_bundle, kefir_ast_type_void()), true,
        nullptr_type));
    ASSERT_NOK(kefir_ast_type_assignable(
        &kft_mem, &global_context.context,
        kefir_ast_type_pointer(&kft_mem, global_context.context.type_bundle, kefir_ast_type_void()), false,
        nullptr_type));
    ASSERT_NOK(kefir_ast_type_assignable(
        &kft_mem, &global_context.context,
        kefir_ast_type_pointer(&kft_mem, global_context.context.type_bundle, kefir_ast_type_signed_int()), true,
        nullptr_type));
    ASSERT_NOK(kefir_ast_type_assignable(
        &kft_mem, &global_context.context,
        kefir_ast_type_pointer(&kft_mem, global_context.context.type_bundle, kefir_ast_type_signed_int()), false,
        nullptr_type));

    ASSERT_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
}
END_CASE

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
#include "kefir/ast/global_context.h"

#define ASSERT_DESIGNATOR_OFFSET(_mem, _env, _type, _designator, _size, _alignment, _offset, _max_bitfield)   \
    do {                                                                                                      \
        struct kefir_ast_designator *designator = (_designator);                                              \
        struct kefir_ast_target_environment_object_info type_info;                                            \
        ASSERT_OK(KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_INFO((_mem), (_env), (_type), designator, &type_info)); \
        if (designator != NULL) {                                                                             \
            ASSERT_OK(kefir_ast_designator_free((_mem), designator));                                         \
        }                                                                                                     \
        ASSERT(type_info.size == (_size));                                                                    \
        ASSERT(type_info.alignment == (_alignment));                                                          \
        ASSERT(type_info.relative_offset == (_offset));                                                       \
        ASSERT(type_info.max_bitfield_width == (_max_bitfield));                                              \
    } while (0)

DEFINE_CASE(ast_translator_environment1, "AST translator - environment object info") {
    struct kefir_string_pool symbols;
    struct kefir_ast_type_bundle type_bundle;
    struct kefir_ast_translator_environment env;
    struct kefir_ast_global_context global_context;

    ASSERT_OK(kefir_string_pool_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));
    ASSERT_OK(kefir_ast_translator_environment_init(&env, kft_util_get_ir_target_platform()));
    REQUIRE_OK(kefir_ast_global_context_init(&kft_mem, kefir_util_default_type_traits(), &env.target_env,
                                             &global_context, NULL));

    struct kefir_ast_struct_type *struct1_type = NULL;
    const struct kefir_ast_type *type1 = kefir_ast_type_structure(&kft_mem, &type_bundle, "", &struct1_type);
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, &symbols, struct1_type, "x", kefir_ast_type_char(), NULL));
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, &symbols, struct1_type, "y", kefir_ast_type_float(), NULL));
    ASSERT_OK(kefir_ast_struct_type_field(
        &kft_mem, &symbols, struct1_type, "z",
        kefir_ast_type_array(&kft_mem, &type_bundle, kefir_ast_type_signed_short(), 16, NULL), NULL));

    struct kefir_ast_struct_type *struct2_type = NULL;
    const struct kefir_ast_type *type2 = kefir_ast_type_structure(&kft_mem, &type_bundle, "", &struct2_type);
    ASSERT_OK(
        kefir_ast_struct_type_field(&kft_mem, &symbols, struct2_type, "field1", kefir_ast_type_signed_long(), NULL));
    ASSERT_OK(
        kefir_ast_struct_type_field(&kft_mem, &symbols, struct2_type, "field2", kefir_ast_type_signed_int(), NULL));
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, &symbols, struct2_type, NULL, type1, NULL));
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, &symbols, struct2_type, "field3", type1, NULL));

    kefir_ast_target_environment_opaque_type_t opaque_type;
    ASSERT_OK(KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE(&kft_mem, &global_context.context, &env.target_env, type2,
                                                    &opaque_type, NULL));

    ASSERT_DESIGNATOR_OFFSET(&kft_mem, &env.target_env, opaque_type, NULL, 96, 8, 0, 0);

    ASSERT_DESIGNATOR_OFFSET(&kft_mem, &env.target_env, opaque_type,
                             kefir_ast_new_member_designator(&kft_mem, &symbols, "field1", NULL), 8, 8, 0, 64);

    ASSERT_DESIGNATOR_OFFSET(&kft_mem, &env.target_env, opaque_type,
                             kefir_ast_new_member_designator(&kft_mem, &symbols, "field2", NULL), 4, 4, 8, 32);

    ASSERT_DESIGNATOR_OFFSET(&kft_mem, &env.target_env, opaque_type,
                             kefir_ast_new_member_designator(&kft_mem, &symbols, "x", NULL), 1, 1, 12, 8);

    ASSERT_DESIGNATOR_OFFSET(&kft_mem, &env.target_env, opaque_type,
                             kefir_ast_new_member_designator(&kft_mem, &symbols, "y", NULL), 4, 4, 16, 0);

    ASSERT_DESIGNATOR_OFFSET(&kft_mem, &env.target_env, opaque_type,
                             kefir_ast_new_member_designator(&kft_mem, &symbols, "z", NULL), 32, 2, 20, 0);

    for (kefir_size_t i = 0; i < 16; i++) {
        ASSERT_DESIGNATOR_OFFSET(
            &kft_mem, &env.target_env, opaque_type,
            kefir_ast_new_index_designator(&kft_mem, i, kefir_ast_new_member_designator(&kft_mem, &symbols, "z", NULL)),
            2, 2, 20 + i * 2, 16);
    }

    ASSERT_DESIGNATOR_OFFSET(&kft_mem, &env.target_env, opaque_type,
                             kefir_ast_new_member_designator(&kft_mem, &symbols, "field3", NULL), 40, 4, 52, 0);

    ASSERT_DESIGNATOR_OFFSET(
        &kft_mem, &env.target_env, opaque_type,
        kefir_ast_new_member_designator(&kft_mem, &symbols, "x",
                                        kefir_ast_new_member_designator(&kft_mem, &symbols, "field3", NULL)),
        1, 1, 52, 8);

    ASSERT_DESIGNATOR_OFFSET(
        &kft_mem, &env.target_env, opaque_type,
        kefir_ast_new_member_designator(&kft_mem, &symbols, "y",
                                        kefir_ast_new_member_designator(&kft_mem, &symbols, "field3", NULL)),
        4, 4, 56, 0);

    ASSERT_DESIGNATOR_OFFSET(
        &kft_mem, &env.target_env, opaque_type,
        kefir_ast_new_member_designator(&kft_mem, &symbols, "z",
                                        kefir_ast_new_member_designator(&kft_mem, &symbols, "field3", NULL)),
        32, 2, 60, 0);

    for (kefir_size_t i = 0; i < 16; i++) {
        ASSERT_DESIGNATOR_OFFSET(
            &kft_mem, &env.target_env, opaque_type,
            kefir_ast_new_index_designator(
                &kft_mem, i,
                kefir_ast_new_member_designator(&kft_mem, &symbols, "z",
                                                kefir_ast_new_member_designator(&kft_mem, &symbols, "field3", NULL))),
            2, 2, 60 + i * 2, 16);
    }

    ASSERT_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE(&kft_mem, &env.target_env, opaque_type));

    REQUIRE_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE

#undef ASSERT_DESIGNATOR_OFFSET

#define ASSERT_OBJECT_OFFSET(_mem, _context, _env, _type, _min, _max, _size)                                       \
    do {                                                                                                           \
        kefir_ast_target_environment_opaque_type_t opaque_type;                                                    \
        ASSERT_OK(KEFIR_AST_TARGET_ENVIRONMENT_GET_TYPE((_mem), (_context), (_env), (_type), &opaque_type, NULL)); \
        for (kefir_int64_t i = (_min); i < (_max); i++) {                                                          \
            kefir_int64_t offset;                                                                                  \
            ASSERT_OK(KEFIR_AST_TARGET_ENVIRONMENT_OBJECT_OFFSET((_mem), (_env), opaque_type, i, &offset));        \
            ASSERT(i *(_size) == offset);                                                                          \
        }                                                                                                          \
        ASSERT_OK(KEFIR_AST_TARGET_ENVIRONMENT_FREE_TYPE((_mem), (_env), opaque_type));                            \
    } while (0)

DEFINE_CASE(ast_translator_environment2, "AST translator - environment object offset") {
    struct kefir_string_pool symbols;
    struct kefir_ast_type_bundle type_bundle;
    struct kefir_ast_translator_environment env;
    struct kefir_ast_global_context global_context;

    ASSERT_OK(kefir_string_pool_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));
    ASSERT_OK(kefir_ast_translator_environment_init(&env, kft_util_get_ir_target_platform()));
    REQUIRE_OK(kefir_ast_global_context_init(&kft_mem, kefir_util_default_type_traits(), &env.target_env,
                                             &global_context, NULL));

    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_char(), -100, 100, 1);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_unsigned_char(), -100, 100,
                         1);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_signed_char(), -100, 100,
                         1);

    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_signed_short(), -100, 100,
                         2);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_unsigned_short(), -100, 100,
                         2);

    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_signed_int(), -100, 100, 4);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_unsigned_int(), -100, 100,
                         4);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_float(), -100, 100, 4);

    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_signed_long(), -100, 100,
                         8);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_unsigned_long(), -100, 100,
                         8);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_signed_long_long(), -100,
                         100, 8);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_unsigned_long_long(), -100,
                         100, 8);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, kefir_ast_type_double(), -100, 100, 8);

    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env,
                         kefir_ast_type_pointer(&kft_mem, &type_bundle, kefir_ast_type_void()), -100, 100, 8);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env,
                         kefir_ast_type_pointer(&kft_mem, &type_bundle, kefir_ast_type_char()), -100, 100, 8);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env,
                         kefir_ast_type_pointer(&kft_mem, &type_bundle, kefir_ast_type_signed_int()), -100, 100, 8);
    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env,
                         kefir_ast_type_pointer(&kft_mem, &type_bundle, kefir_ast_type_double()), -100, 100, 8);
    ASSERT_OBJECT_OFFSET(
        &kft_mem, &global_context.context, &env.target_env,
        kefir_ast_type_pointer(&kft_mem, &type_bundle,
                               kefir_ast_type_pointer(&kft_mem, &type_bundle, kefir_ast_type_unsigned_short())),
        -100, 100, 8);

    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env,
                         kefir_ast_type_array(&kft_mem, &type_bundle, kefir_ast_type_unsigned_char(), 16, NULL), -100,
                         100, 16);

    struct kefir_ast_struct_type *struct_type1 = NULL;
    const struct kefir_ast_type *type1 = kefir_ast_type_structure(&kft_mem, &type_bundle, "", &struct_type1);
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, &symbols, struct_type1, "x", kefir_ast_type_unsigned_long(), NULL));
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, &symbols, struct_type1, "y",
                                          kefir_ast_type_pointer(&kft_mem, &type_bundle, kefir_ast_type_boolean()),
                                          NULL));
    ASSERT_OK(kefir_ast_struct_type_field(&kft_mem, &symbols, struct_type1, "z",
                                          kefir_ast_type_array(&kft_mem, &type_bundle, kefir_ast_type_float(), 5, NULL),
                                          NULL));

    ASSERT_OBJECT_OFFSET(&kft_mem, &global_context.context, &env.target_env, type1, -100, 100, 40);

    REQUIRE_OK(kefir_ast_global_context_free(&kft_mem, &global_context));
    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE

#undef ASSERT_OBJECT_OFFSET

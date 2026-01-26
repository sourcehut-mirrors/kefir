/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
#include "kefir/ast/type_conv.h"

DEFINE_CASE(ast_int128_type1, "AST Type analysis - int128 type #1") {
    struct kefir_string_pool symbols;
    struct kefir_ast_type_bundle type_bundle;
    ASSERT_OK(kefir_string_pool_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();

    const struct kefir_ast_type *signed_type = kefir_ast_type_signed_int128();
    const struct kefir_ast_type *unsigned_type = kefir_ast_type_unsigned_int128();
    
    ASSERT(signed_type != NULL);
    ASSERT(signed_type->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_INT128);
    ASSERT(KEFIR_AST_TYPE_IS_EXTENDED_SIGNED_INTEGER(signed_type));
    ASSERT(KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(signed_type));
    ASSERT(KEFIR_INTERNAL_AST_TYPE_IS_SIGNED_INTEGER(signed_type));

    ASSERT(unsigned_type != NULL);
    ASSERT(unsigned_type->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT128);
    ASSERT(KEFIR_AST_TYPE_IS_EXTENDED_UNSIGNED_INTEGER(unsigned_type));
    ASSERT(KEFIR_AST_TYPE_IS_EXTENDED_INTEGER(unsigned_type));
    ASSERT(KEFIR_INTERNAL_AST_TYPE_IS_UNSIGNED_INTEGER(unsigned_type));

    kefir_size_t signed_rank, unsigned_rank, rank;
    ASSERT_OK(type_traits->integral_type_rank(type_traits, signed_type, &signed_rank));
    ASSERT_OK(type_traits->integral_type_rank(type_traits, unsigned_type, &unsigned_rank));
    ASSERT(signed_rank == unsigned_rank);

    ASSERT_OK(type_traits->integral_type_rank(type_traits, kefir_ast_type_char(), &rank));
    ASSERT(signed_rank > rank);
    ASSERT_OK(type_traits->integral_type_rank(type_traits, kefir_ast_type_signed_short(), &rank));
    ASSERT(signed_rank > rank);
    ASSERT_OK(type_traits->integral_type_rank(type_traits, kefir_ast_type_signed_int(), &rank));
    ASSERT(signed_rank > rank);
    ASSERT_OK(type_traits->integral_type_rank(type_traits, kefir_ast_type_signed_long(), &rank));
    ASSERT(signed_rank > rank);
    ASSERT_OK(type_traits->integral_type_rank(type_traits, kefir_ast_type_signed_long_long(), &rank));
    ASSERT(signed_rank > rank);

    const struct kefir_ast_type *bitint127_type = kefir_ast_type_signed_bitprecise(&kft_mem, &type_bundle, 127);
    ASSERT(bitint127_type != NULL);
    ASSERT_OK(type_traits->integral_type_rank(type_traits, bitint127_type, &rank));
    ASSERT(signed_rank > rank);

    const struct kefir_ast_type *bitint128_type = kefir_ast_type_signed_bitprecise(&kft_mem, &type_bundle, 128);
    ASSERT(bitint128_type != NULL);
    ASSERT_OK(type_traits->integral_type_rank(type_traits, bitint128_type, &rank));
    ASSERT(signed_rank > rank);

    const struct kefir_ast_type *bitint129_type = kefir_ast_type_signed_bitprecise(&kft_mem, &type_bundle, 129);
    ASSERT(bitint129_type != NULL);
    ASSERT_OK(type_traits->integral_type_rank(type_traits, bitint129_type, &rank));
    ASSERT(signed_rank < rank);

    ASSERT(KEFIR_AST_TYPE_SAME(signed_type, signed_type));
    ASSERT(KEFIR_AST_TYPE_SAME(unsigned_type, unsigned_type));
    ASSERT(!KEFIR_AST_TYPE_SAME(signed_type, unsigned_type));
    ASSERT(!KEFIR_AST_TYPE_SAME(unsigned_type, signed_type));

    ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, signed_type));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, unsigned_type));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, kefir_ast_type_signed_char()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, kefir_ast_type_unsigned_char()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, kefir_ast_type_signed_short()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, kefir_ast_type_unsigned_short()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, kefir_ast_type_signed_int()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, kefir_ast_type_unsigned_int()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, kefir_ast_type_signed_long()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, kefir_ast_type_unsigned_long()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, kefir_ast_type_signed_long_long()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, kefir_ast_type_unsigned_long_long()));

    ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, unsigned_type));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, signed_type));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, kefir_ast_type_signed_char()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, kefir_ast_type_unsigned_char()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, kefir_ast_type_signed_short()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, kefir_ast_type_unsigned_short()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, kefir_ast_type_signed_int()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, kefir_ast_type_unsigned_int()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, kefir_ast_type_signed_long()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, kefir_ast_type_unsigned_long()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, kefir_ast_type_signed_long_long()));
    ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, kefir_ast_type_unsigned_long_long()));

    const struct kefir_ast_type *composite_type = KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, signed_type);
    ASSERT(KEFIR_AST_TYPE_SAME(signed_type, composite_type));
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, kefir_ast_type_signed_char()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, kefir_ast_type_unsigned_char()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, kefir_ast_type_signed_short()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, kefir_ast_type_unsigned_short()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, kefir_ast_type_signed_int()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, kefir_ast_type_unsigned_int()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, kefir_ast_type_signed_long()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, kefir_ast_type_unsigned_long()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, kefir_ast_type_signed_long_long()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, kefir_ast_type_unsigned_long_long()) == NULL);

    composite_type = KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, unsigned_type);
    ASSERT(KEFIR_AST_TYPE_SAME(unsigned_type, composite_type));
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, kefir_ast_type_signed_char()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, kefir_ast_type_unsigned_char()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, kefir_ast_type_signed_short()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, kefir_ast_type_unsigned_short()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, kefir_ast_type_signed_int()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, kefir_ast_type_unsigned_int()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, kefir_ast_type_signed_long()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, kefir_ast_type_unsigned_long()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, kefir_ast_type_signed_long_long()) == NULL);
    ASSERT(KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, kefir_ast_type_unsigned_long_long()) == NULL);

    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE

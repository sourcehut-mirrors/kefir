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
#include "kefir/ast/type_conv.h"

DEFINE_CASE(ast_bit_precise_types1, "AST Type analysis - bit-precise types #1") {
    struct kefir_string_pool symbols;
    struct kefir_ast_type_bundle type_bundle;
    ASSERT_OK(kefir_string_pool_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();

    const struct kefir_ast_type *signed_types[64];
    const struct kefir_ast_type *unsigned_types[64];
    for (kefir_size_t i = 0; i < 64; i++) {
        const struct kefir_ast_type *signed_type = kefir_ast_type_signed_bitprecise(&kft_mem, &type_bundle, i);
        const struct kefir_ast_type *unsigned_type = kefir_ast_type_unsigned_bitprecise(&kft_mem, &type_bundle, i);

        ASSERT(signed_type->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE);
        ASSERT(signed_type->bitprecise.width == i);
        ASSERT(signed_type->bitprecise.flipped_sign_type != NULL);
        ASSERT(signed_type->bitprecise.flipped_sign_type->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE);
        ASSERT(signed_type->bitprecise.flipped_sign_type->bitprecise.width == i);
        ASSERT(signed_type->bitprecise.flipped_sign_type->bitprecise.flipped_sign_type == signed_type);

        ASSERT(unsigned_type->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE);
        ASSERT(unsigned_type->bitprecise.width == i);
        ASSERT(unsigned_type->bitprecise.flipped_sign_type != NULL);
        ASSERT(unsigned_type->bitprecise.flipped_sign_type->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE);
        ASSERT(unsigned_type->bitprecise.flipped_sign_type->bitprecise.width == i);
        ASSERT(unsigned_type->bitprecise.flipped_sign_type->bitprecise.flipped_sign_type == unsigned_type);

        ASSERT(KEFIR_AST_TYPE_SAME(signed_type, signed_type));
        ASSERT(!KEFIR_AST_TYPE_SAME(signed_type, unsigned_type));
        ASSERT(KEFIR_AST_TYPE_SAME(signed_type, unsigned_type->bitprecise.flipped_sign_type));
        ASSERT(KEFIR_AST_TYPE_SAME(unsigned_type, unsigned_type));
        ASSERT(!KEFIR_AST_TYPE_SAME(unsigned_type, signed_type));
        ASSERT(KEFIR_AST_TYPE_SAME(unsigned_type, signed_type->bitprecise.flipped_sign_type));

        ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, signed_type));
        ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, unsigned_type));
        ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, unsigned_type->bitprecise.flipped_sign_type));
        ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, unsigned_type));
        ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, signed_type));
        ASSERT(KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, signed_type->bitprecise.flipped_sign_type));

        const struct kefir_ast_type *composite1 =
            KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, signed_type);
        ASSERT(composite1 != NULL);
        ASSERT(KEFIR_AST_TYPE_SAME(signed_type, composite1));

        const struct kefir_ast_type *composite2 =
            KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, unsigned_type);
        ASSERT(composite2 != NULL);
        ASSERT(KEFIR_AST_TYPE_SAME(unsigned_type, composite2));

        const struct kefir_ast_type *composite3 =
            KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, unsigned_type);
        ASSERT(composite3 == NULL);

        const struct kefir_ast_type *composite4 =
            KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, signed_type);
        ASSERT(composite4 == NULL);

        for (kefir_size_t j = 0; j < i; j++) {
            ASSERT(!KEFIR_AST_TYPE_SAME(signed_type, signed_types[j]));
            ASSERT(!KEFIR_AST_TYPE_SAME(signed_type, unsigned_types[j]));
            ASSERT(!KEFIR_AST_TYPE_SAME(unsigned_type, signed_types[j]));
            ASSERT(!KEFIR_AST_TYPE_SAME(unsigned_type, unsigned_types[j]));

            ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, signed_types[j]));
            ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, unsigned_types[j]));
            ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, signed_types[j]));
            ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, unsigned_types[j]));

            const struct kefir_ast_type *composite5 =
                KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, signed_types[j]);
            ASSERT(composite5 == NULL);

            const struct kefir_ast_type *composite6 =
                KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, unsigned_types[j]);
            ASSERT(composite6 == NULL);

            const struct kefir_ast_type *composite7 =
                KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, unsigned_types[j]);
            ASSERT(composite7 == NULL);

            const struct kefir_ast_type *composite8 =
                KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, signed_types[j]);
            ASSERT(composite8 == NULL);
        }

        signed_types[i] = signed_type;
        unsigned_types[i] = unsigned_type;
    }

    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_bit_precise_types2, "AST Type analysis - bit-precise types #2") {
    struct kefir_string_pool symbols;
    struct kefir_ast_type_bundle type_bundle;
    ASSERT_OK(kefir_string_pool_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();

    const struct kefir_ast_type *other_types[] = {
        kefir_ast_type_void(),
        kefir_ast_type_boolean(),
        kefir_ast_type_char(),
        kefir_ast_type_signed_char(),
        kefir_ast_type_unsigned_char(),
        kefir_ast_type_signed_short(),
        kefir_ast_type_unsigned_short(),
        kefir_ast_type_signed_int(),
        kefir_ast_type_unsigned_int(),
        kefir_ast_type_signed_long(),
        kefir_ast_type_unsigned_long(),
        kefir_ast_type_signed_long_long(),
        kefir_ast_type_unsigned_long_long(),
        kefir_ast_type_float(),
        kefir_ast_type_double(),
        kefir_ast_type_long_double(),
        kefir_ast_type_complex_float(),
        kefir_ast_type_complex_double(),
        kefir_ast_type_complex_long_double(),
        kefir_ast_type_pointer(&kft_mem, &type_bundle, kefir_ast_type_void()),
        kefir_ast_type_qualified(&kft_mem, &type_bundle, kefir_ast_type_char(),
                                 (struct kefir_ast_type_qualification) {.constant = true})};

    for (kefir_size_t i = 0; i < 64; i++) {
        const struct kefir_ast_type *signed_type = kefir_ast_type_signed_bitprecise(&kft_mem, &type_bundle, i);
        const struct kefir_ast_type *unsigned_type = kefir_ast_type_unsigned_bitprecise(&kft_mem, &type_bundle, i);

        for (kefir_size_t j = 0; j < sizeof(other_types) / sizeof(other_types[0]); j++) {
            ASSERT(!KEFIR_AST_TYPE_SAME(signed_type, other_types[j]));
            ASSERT(!KEFIR_AST_TYPE_SAME(unsigned_type, other_types[j]));

            ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, signed_type, other_types[j]));
            ASSERT(!KEFIR_AST_TYPE_COMPATIBLE(type_traits, unsigned_type, other_types[j]));

            const struct kefir_ast_type *composite1 =
                KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, signed_type, other_types[j]);
            ASSERT(composite1 == NULL);

            const struct kefir_ast_type *composite2 =
                KEFIR_AST_TYPE_COMPOSITE(&kft_mem, &type_bundle, type_traits, unsigned_type, other_types[j]);
            ASSERT(composite2 == NULL);
        }
    }

    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_bit_precise_types3, "AST Type analysis - bit-precise types #3") {
    struct kefir_string_pool symbols;
    struct kefir_ast_type_bundle type_bundle;
    ASSERT_OK(kefir_string_pool_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();

    for (kefir_size_t i = 0; i < 64; i++) {
        const struct kefir_ast_type *signed_type = kefir_ast_type_signed_bitprecise(&kft_mem, &type_bundle, i);
        const struct kefir_ast_type *unsigned_type = kefir_ast_type_unsigned_bitprecise(&kft_mem, &type_bundle, i);

        const struct kefir_ast_type *promoted1 = kefir_ast_type_int_promotion(
            type_traits, signed_type, (struct kefir_ast_bitfield_properties) {.bitfield = false});
        ASSERT(promoted1 != NULL);
        ASSERT(KEFIR_AST_TYPE_SAME(signed_type, promoted1));

        const struct kefir_ast_type *promoted2 = kefir_ast_type_int_promotion(
            type_traits, unsigned_type, (struct kefir_ast_bitfield_properties) {.bitfield = false});
        ASSERT(promoted2 != NULL);
        ASSERT(KEFIR_AST_TYPE_SAME(unsigned_type, promoted2));

        const struct kefir_ast_type *promoted3 = kefir_ast_type_int_promotion(
            type_traits, signed_type,
            (struct kefir_ast_bitfield_properties) {.bitfield = true, .width = i > 0 ? i - 1 : i});
        ASSERT(promoted3 != NULL);
        ASSERT(KEFIR_AST_TYPE_SAME(signed_type, promoted3));

        const struct kefir_ast_type *promoted4 = kefir_ast_type_int_promotion(
            type_traits, unsigned_type,
            (struct kefir_ast_bitfield_properties) {.bitfield = true, .width = i > 0 ? i - 1 : i});
        ASSERT(promoted4 != NULL);
        ASSERT(KEFIR_AST_TYPE_SAME(unsigned_type, promoted4));

        const struct kefir_ast_type *promoted5 =
            type_traits->bitfield_promotion(type_traits, signed_type, i > 0 ? i - 1 : i);
        ASSERT(KEFIR_AST_TYPE_SAME(signed_type, promoted5));

        const struct kefir_ast_type *promoted6 =
            type_traits->bitfield_promotion(type_traits, unsigned_type, i > 0 ? i - 1 : i);
        ASSERT(KEFIR_AST_TYPE_SAME(unsigned_type, promoted6));
    }

    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_bit_precise_types4, "AST Type analysis - bit-precise types #4") {
    struct kefir_string_pool symbols;
    struct kefir_ast_type_bundle type_bundle;
    ASSERT_OK(kefir_string_pool_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();

    const struct kefir_ast_type *other_types[] = {
        kefir_ast_type_boolean(),          kefir_ast_type_char(),
        kefir_ast_type_signed_char(),      kefir_ast_type_unsigned_char(),
        kefir_ast_type_signed_short(),     kefir_ast_type_unsigned_short(),
        kefir_ast_type_signed_int(),       kefir_ast_type_unsigned_int(),
        kefir_ast_type_signed_long(),      kefir_ast_type_unsigned_long(),
        kefir_ast_type_signed_long_long(), kefir_ast_type_unsigned_long_long()};

    for (kefir_size_t i = 0; i < 64; i++) {
        const struct kefir_ast_type *signed_type = kefir_ast_type_signed_bitprecise(&kft_mem, &type_bundle, i);
        const struct kefir_ast_type *unsigned_type = kefir_ast_type_unsigned_bitprecise(&kft_mem, &type_bundle, i);

        for (kefir_size_t j = 0; j < sizeof(other_types) / sizeof(other_types[0]); j++) {
            const struct kefir_ast_type *common_type1 = kefir_ast_type_common_arithmetic(
                type_traits, signed_type, (struct kefir_ast_bitfield_properties) {.bitfield = false}, other_types[j],
                (struct kefir_ast_bitfield_properties) {.bitfield = false});
            ASSERT(common_type1 != NULL);

            const struct kefir_ast_type *common_type2 = kefir_ast_type_common_arithmetic(
                type_traits, unsigned_type, (struct kefir_ast_bitfield_properties) {.bitfield = false}, other_types[j],
                (struct kefir_ast_bitfield_properties) {.bitfield = false});
            ASSERT(common_type2 != NULL);

            const struct kefir_ast_type *common_type3 = kefir_ast_type_common_arithmetic(
                type_traits, other_types[j], (struct kefir_ast_bitfield_properties) {.bitfield = false}, signed_type,
                (struct kefir_ast_bitfield_properties) {.bitfield = false});
            ASSERT(common_type3 != NULL);

            const struct kefir_ast_type *common_type4 = kefir_ast_type_common_arithmetic(
                type_traits, other_types[j], (struct kefir_ast_bitfield_properties) {.bitfield = false}, unsigned_type,
                (struct kefir_ast_bitfield_properties) {.bitfield = false});
            ASSERT(common_type4 != NULL);

            if (other_types[j]->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_LONG ||
                other_types[j]->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG ||
                other_types[j]->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG ||
                other_types[j]->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG) {
                ASSERT(KEFIR_AST_TYPE_SAME(common_type1, other_types[j]));
                ASSERT(KEFIR_AST_TYPE_SAME(common_type2, other_types[j]));
                ASSERT(KEFIR_AST_TYPE_SAME(common_type3, other_types[j]));
                ASSERT(KEFIR_AST_TYPE_SAME(common_type4, other_types[j]));
            } else {
                if (i <= 32) {
                    ASSERT(KEFIR_AST_TYPE_SAME(
                        common_type1,
                        kefir_ast_type_int_promotion(type_traits, other_types[j],
                                                     (struct kefir_ast_bitfield_properties) {.bitfield = false})));
                    ASSERT(KEFIR_AST_TYPE_SAME(
                        common_type3,
                        kefir_ast_type_int_promotion(type_traits, other_types[j],
                                                     (struct kefir_ast_bitfield_properties) {.bitfield = false})));

                } else {
                    ASSERT(KEFIR_AST_TYPE_SAME(common_type1, signed_type));
                    ASSERT(KEFIR_AST_TYPE_SAME(common_type3, signed_type));
                }

                if (i < 32) {
                    ASSERT(KEFIR_AST_TYPE_SAME(
                        common_type2,
                        kefir_ast_type_int_promotion(type_traits, other_types[j],
                                                     (struct kefir_ast_bitfield_properties) {.bitfield = false})));
                    ASSERT(KEFIR_AST_TYPE_SAME(
                        common_type4,
                        kefir_ast_type_int_promotion(type_traits, other_types[j],
                                                     (struct kefir_ast_bitfield_properties) {.bitfield = false})));
                } else if (i == 32) {
                    ASSERT(KEFIR_AST_TYPE_SAME(common_type2, kefir_ast_type_unsigned_int()));
                    ASSERT(KEFIR_AST_TYPE_SAME(common_type4, kefir_ast_type_unsigned_int()));
                } else {
                    ASSERT(KEFIR_AST_TYPE_SAME(common_type2, unsigned_type));
                    ASSERT(KEFIR_AST_TYPE_SAME(common_type4, unsigned_type));
                }
            }
        }
    }

    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE

DEFINE_CASE(ast_bit_precise_types5, "AST Type analysis - bit-precise types #5") {
    struct kefir_string_pool symbols;
    struct kefir_ast_type_bundle type_bundle;
    ASSERT_OK(kefir_string_pool_init(&symbols));
    ASSERT_OK(kefir_ast_type_bundle_init(&type_bundle, &symbols));
    const struct kefir_ast_type_traits *type_traits = kefir_util_default_type_traits();

    const struct kefir_ast_type *signed_types[64];
    const struct kefir_ast_type *unsigned_types[64];

    for (kefir_size_t i = 0; i < 64; i++) {
        const struct kefir_ast_type *signed_type = kefir_ast_type_signed_bitprecise(&kft_mem, &type_bundle, i);
        const struct kefir_ast_type *unsigned_type = kefir_ast_type_unsigned_bitprecise(&kft_mem, &type_bundle, i);

        for (kefir_size_t j = 0; j < i; j++) {
            const struct kefir_ast_type *common_type1 = kefir_ast_type_common_arithmetic(
                type_traits, signed_type, (struct kefir_ast_bitfield_properties) {.bitfield = false}, signed_types[j],
                (struct kefir_ast_bitfield_properties) {.bitfield = false});
            ASSERT(common_type1 != NULL);

            const struct kefir_ast_type *common_type2 = kefir_ast_type_common_arithmetic(
                type_traits, unsigned_type, (struct kefir_ast_bitfield_properties) {.bitfield = false},
                unsigned_types[j], (struct kefir_ast_bitfield_properties) {.bitfield = false});
            ASSERT(common_type2 != NULL);

            const struct kefir_ast_type *common_type3 = kefir_ast_type_common_arithmetic(
                type_traits, signed_types[j], (struct kefir_ast_bitfield_properties) {.bitfield = false}, signed_type,
                (struct kefir_ast_bitfield_properties) {.bitfield = false});
            ASSERT(common_type3 != NULL);

            const struct kefir_ast_type *common_type4 = kefir_ast_type_common_arithmetic(
                type_traits, unsigned_types[j], (struct kefir_ast_bitfield_properties) {.bitfield = false},
                unsigned_type, (struct kefir_ast_bitfield_properties) {.bitfield = false});
            ASSERT(common_type4 != NULL);

            ASSERT(KEFIR_AST_TYPE_SAME(common_type1, signed_type));
            ASSERT(KEFIR_AST_TYPE_SAME(common_type2, unsigned_type));
            ASSERT(KEFIR_AST_TYPE_SAME(common_type3, signed_type));
            ASSERT(KEFIR_AST_TYPE_SAME(common_type4, unsigned_type));
        }

        signed_types[i] = signed_type;
        unsigned_types[i] = unsigned_type;
    }

    ASSERT_OK(kefir_ast_type_bundle_free(&kft_mem, &type_bundle));
    ASSERT_OK(kefir_string_pool_free(&kft_mem, &symbols));
}
END_CASE

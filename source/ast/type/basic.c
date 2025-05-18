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

#include "kefir/ast/type.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

static kefir_bool_t same_basic_type(const struct kefir_ast_type *type1, const struct kefir_ast_type *type2) {
    REQUIRE(type1 != NULL, false);
    REQUIRE(type2 != NULL, false);
    REQUIRE(type1->tag == type2->tag, false);
    if (type1->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE ||
        type1->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE) {
        REQUIRE(type1->bitprecise.width == type2->bitprecise.width, false);
    }
    return true;
}

static kefir_bool_t compatible_basic_types(const struct kefir_ast_type_traits *type_traits,
                                           const struct kefir_ast_type *type1, const struct kefir_ast_type *type2) {
    UNUSED(type_traits);
    REQUIRE(type1 != NULL, false);
    REQUIRE(type2 != NULL, false);
    if (type1->tag == KEFIR_AST_TYPE_ENUMERATION) {
        return compatible_basic_types(type_traits, kefir_ast_enumeration_underlying_type(&type1->enumeration_type),
                                      type2);
    } else if (type2->tag == KEFIR_AST_TYPE_ENUMERATION) {
        return compatible_basic_types(type_traits, type1,
                                      kefir_ast_enumeration_underlying_type(&type2->enumeration_type));
    }
    REQUIRE(type1->tag == type2->tag, false);
    if (type1->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE ||
        type1->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE) {
        REQUIRE(type1->bitprecise.width == type2->bitprecise.width, false);
    }
    return true;
}

const struct kefir_ast_type *composite_basic_types(struct kefir_mem *mem, struct kefir_ast_type_bundle *type_bundle,
                                                   const struct kefir_ast_type_traits *type_traits,
                                                   const struct kefir_ast_type *type1,
                                                   const struct kefir_ast_type *type2) {
    UNUSED(mem);
    UNUSED(type_bundle);
    REQUIRE(type_traits != NULL, NULL);
    REQUIRE(type1 != NULL, NULL);
    REQUIRE(type2 != NULL, NULL);
    REQUIRE(KEFIR_AST_TYPE_COMPATIBLE(type_traits, type1, type2), NULL);
    if (type2->tag == KEFIR_AST_TYPE_ENUMERATION) {
        return type2;
    } else {
        return type1;
    }
}

static kefir_result_t free_nothing(struct kefir_mem *mem, const struct kefir_ast_type *type) {
    UNUSED(mem);
    UNUSED(type);
    return KEFIR_OK;
}

static const struct kefir_ast_type SCALAR_VOID = {.tag = KEFIR_AST_TYPE_VOID,
                                                  .ops = {.same = same_basic_type,
                                                          .compatible = compatible_basic_types,
                                                          .composite = composite_basic_types,
                                                          .free = free_nothing}};

const struct kefir_ast_type *kefir_ast_type_void(void) {
    return &SCALAR_VOID;
}

static const struct kefir_ast_type AUTO_TYPE = {.tag = KEFIR_AST_TYPE_AUTO,
                                                .ops = {.same = same_basic_type,
                                                        .compatible = compatible_basic_types,
                                                        .composite = composite_basic_types,
                                                        .free = free_nothing}};

const struct kefir_ast_type *kefir_ast_type_auto(void) {
    return &AUTO_TYPE;
}

#define SCALAR_TYPE(id, _tag)                                                                               \
    static const struct kefir_ast_type DEFAULT_SCALAR_##id = {.tag = (_tag),                                \
                                                              .ops = {.same = same_basic_type,              \
                                                                      .compatible = compatible_basic_types, \
                                                                      .composite = composite_basic_types,   \
                                                                      .free = free_nothing}};               \
                                                                                                            \
    const struct kefir_ast_type *kefir_ast_type_##id(void) {                                                \
        return &DEFAULT_SCALAR_##id;                                                                        \
    }

SCALAR_TYPE(boolean, KEFIR_AST_TYPE_SCALAR_BOOL)
SCALAR_TYPE(char, KEFIR_AST_TYPE_SCALAR_CHAR)
SCALAR_TYPE(unsigned_char, KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR)
SCALAR_TYPE(signed_char, KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR)
SCALAR_TYPE(unsigned_short, KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT)
SCALAR_TYPE(signed_short, KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT)
SCALAR_TYPE(unsigned_int, KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT)
SCALAR_TYPE(signed_int, KEFIR_AST_TYPE_SCALAR_SIGNED_INT)
SCALAR_TYPE(unsigned_long, KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG)
SCALAR_TYPE(signed_long, KEFIR_AST_TYPE_SCALAR_SIGNED_LONG)
SCALAR_TYPE(unsigned_long_long, KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG)
SCALAR_TYPE(signed_long_long, KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG)
SCALAR_TYPE(float, KEFIR_AST_TYPE_SCALAR_FLOAT)
SCALAR_TYPE(double, KEFIR_AST_TYPE_SCALAR_DOUBLE)
SCALAR_TYPE(long_double, KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE)

#undef SCALAR_TYPE

#define COMPLEX_TYPE(id, _tag)                                                                               \
    static const struct kefir_ast_type DEFAULT_COMPLEX_##id = {.tag = (_tag),                                \
                                                               .ops = {.same = same_basic_type,              \
                                                                       .compatible = compatible_basic_types, \
                                                                       .composite = composite_basic_types,   \
                                                                       .free = free_nothing}};               \
                                                                                                             \
    const struct kefir_ast_type *kefir_ast_type_complex_##id(void) {                                         \
        return &DEFAULT_COMPLEX_##id;                                                                        \
    }

COMPLEX_TYPE(float, KEFIR_AST_TYPE_COMPLEX_FLOAT)
COMPLEX_TYPE(double, KEFIR_AST_TYPE_COMPLEX_DOUBLE)
COMPLEX_TYPE(long_double, KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE)

#undef COMPLEX_TYPE

static kefir_result_t free_bitprecise(struct kefir_mem *mem, const struct kefir_ast_type *type) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));

    ASSIGN_DECL_CAST(void *, type_ptr, type);
    memset(type_ptr, 0, sizeof(struct kefir_ast_type));
    KEFIR_FREE(mem, type_ptr);
    return KEFIR_OK;
}

const struct kefir_ast_type *kefir_ast_type_signed_bitprecise(struct kefir_mem *mem,
                                                              struct kefir_ast_type_bundle *type_bundle,
                                                              kefir_size_t width) {
    REQUIRE(mem != NULL, NULL);
    REQUIRE(type_bundle != NULL, NULL);

    struct kefir_ast_type *signed_type = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_type));
    REQUIRE(signed_type != NULL, NULL);

    struct kefir_ast_type *unsigned_type = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_type));
    REQUIRE_ELSE(unsigned_type != NULL, {
        KEFIR_FREE(mem, signed_type);
        return NULL;
    });

    signed_type->tag = KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE;
    signed_type->ops.same = same_basic_type;
    signed_type->ops.compatible = compatible_basic_types;
    signed_type->ops.composite = composite_basic_types;
    signed_type->ops.free = free_bitprecise;
    signed_type->bitprecise.width = width;
    signed_type->bitprecise.flipped_sign_type = NULL;

    kefir_result_t res =
        kefir_list_insert_after(mem, &type_bundle->types, kefir_list_tail(&type_bundle->types), signed_type);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, signed_type);
        KEFIR_FREE(mem, unsigned_type);
        return NULL;
    });

    unsigned_type->tag = KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE;
    unsigned_type->ops.same = same_basic_type;
    unsigned_type->ops.compatible = compatible_basic_types;
    unsigned_type->ops.composite = composite_basic_types;
    unsigned_type->ops.free = free_bitprecise;
    unsigned_type->bitprecise.width = width;
    unsigned_type->bitprecise.flipped_sign_type = signed_type;

    res = kefir_list_insert_after(mem, &type_bundle->types, kefir_list_tail(&type_bundle->types), unsigned_type);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, unsigned_type);
        return NULL;
    });

    signed_type->bitprecise.flipped_sign_type = unsigned_type;

    return signed_type;
}

const struct kefir_ast_type *kefir_ast_type_unsigned_bitprecise(struct kefir_mem *mem,
                                                                struct kefir_ast_type_bundle *type_bundle,
                                                                kefir_size_t width) {
    const struct kefir_ast_type *signed_type = kefir_ast_type_signed_bitprecise(mem, type_bundle, width);
    REQUIRE(signed_type != NULL, NULL);
    return signed_type->bitprecise.flipped_sign_type;
}

const struct kefir_ast_type *kefir_ast_type_flip_integer_singedness(const struct kefir_ast_type_traits *type_traits,
                                                                    const struct kefir_ast_type *type) {
    REQUIRE(type_traits != NULL, NULL);
    REQUIRE(type != NULL, NULL);

    switch (type->tag) {
        case KEFIR_AST_TYPE_SCALAR_BOOL:
            return type;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
            if (type_traits->character_type_signedness) {
                return kefir_ast_type_unsigned_char();
            } else {
                return kefir_ast_type_signed_char();
            }

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
            return kefir_ast_type_signed_char();

        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
            return kefir_ast_type_unsigned_char();

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
            return kefir_ast_type_signed_short();

        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
            return kefir_ast_type_unsigned_short();

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
            return kefir_ast_type_signed_int();

        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
            return kefir_ast_type_unsigned_int();

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
            return kefir_ast_type_signed_long();

        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
            return kefir_ast_type_unsigned_long();

        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
            return kefir_ast_type_signed_long_long();

        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
            return kefir_ast_type_unsigned_long_long();

        case KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE:
            return type->bitprecise.flipped_sign_type;

        default:
            return NULL;
    }
}

const struct kefir_ast_type *kefir_ast_type_corresponding_real_type(const struct kefir_ast_type *type) {
    switch (type->tag) {
        case KEFIR_AST_TYPE_COMPLEX_FLOAT:
            return kefir_ast_type_float();

        case KEFIR_AST_TYPE_COMPLEX_DOUBLE:
            return kefir_ast_type_double();

        case KEFIR_AST_TYPE_COMPLEX_LONG_DOUBLE:
            return kefir_ast_type_long_double();

        default:
            if (KEFIR_AST_TYPE_IS_REAL_TYPE(type)) {
                return type;
            } else {
                return NULL;
            }
    }
}

const struct kefir_ast_type *kefir_ast_type_corresponding_complex_type(const struct kefir_ast_type *type) {
    switch (type->tag) {
        case KEFIR_AST_TYPE_SCALAR_FLOAT:
            return kefir_ast_type_complex_float();

        case KEFIR_AST_TYPE_SCALAR_DOUBLE:
            return kefir_ast_type_complex_double();

        case KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE:
            return kefir_ast_type_complex_long_double();

        default:
            if (KEFIR_AST_TYPE_IS_COMPLEX_TYPE(type)) {
                return type;
            } else {
                return NULL;
            }
    }
}

kefir_result_t kefir_ast_type_is_signed(const struct kefir_ast_type_traits *type_traits,
                                        const struct kefir_ast_type *type, kefir_bool_t *signedness) {
    REQUIRE(type_traits != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type traits"));
    REQUIRE(type != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST type"));
    REQUIRE(signedness != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to boolean"));

    switch (type->tag) {
        case KEFIR_AST_TYPE_SCALAR_BOOL:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE:
        case KEFIR_AST_TYPE_SCALAR_POINTER:
            *signedness = false;
            break;

        case KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_INT:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG:
        case KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE:
            *signedness = true;
            break;

        case KEFIR_AST_TYPE_SCALAR_CHAR:
            *signedness = type_traits->character_type_signedness;
            break;

        case KEFIR_AST_TYPE_ENUMERATION:
            REQUIRE_OK(kefir_ast_type_is_signed(type_traits, type->enumeration_type.underlying_type, signedness));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Expected integral AST type");
    }
    return KEFIR_OK;
}

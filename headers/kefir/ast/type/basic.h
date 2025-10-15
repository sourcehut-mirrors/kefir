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

#ifndef KEFIR_AST_TYPE_BASIC_H_
#define KEFIR_AST_TYPE_BASIC_H_

#include "kefir/ast/type/base.h"

typedef struct kefir_ast_type_bit_precise_integer {
    kefir_size_t width;
    const struct kefir_ast_type *flipped_sign_type;
} kefir_ast_type_bit_precise_integer_t;

typedef struct kefir_ast_type_complex {
    const struct kefir_ast_type *real_type;
} kefir_ast_type_complex_t;

typedef struct kefir_ast_type_imaginary {
    const struct kefir_ast_type *real_type;
} kefir_ast_type_imaginary_t;

#define SCALAR_TYPE(id) const struct kefir_ast_type *kefir_ast_type_##id(void)
SCALAR_TYPE(void);
SCALAR_TYPE(auto);
SCALAR_TYPE(boolean);
SCALAR_TYPE(char);
SCALAR_TYPE(unsigned_char);
SCALAR_TYPE(signed_char);
SCALAR_TYPE(unsigned_short);
SCALAR_TYPE(signed_short);
SCALAR_TYPE(unsigned_int);
SCALAR_TYPE(signed_int);
SCALAR_TYPE(unsigned_long);
SCALAR_TYPE(signed_long);
SCALAR_TYPE(unsigned_long_long);
SCALAR_TYPE(signed_long_long);
SCALAR_TYPE(float);
SCALAR_TYPE(double);
SCALAR_TYPE(long_double);
SCALAR_TYPE(interchange_float32);
SCALAR_TYPE(interchange_float64);
SCALAR_TYPE(interchange_float80);
SCALAR_TYPE(extended_float32);
SCALAR_TYPE(extended_float64);
SCALAR_TYPE(decimal32);
SCALAR_TYPE(decimal64);
SCALAR_TYPE(decimal128);
SCALAR_TYPE(extended_decimal64);
SCALAR_TYPE(nullptr);
#undef SCALAR_TYPE

#define COMPLEX_TYPE(id) const struct kefir_ast_type *kefir_ast_type_complex_##id(void)
COMPLEX_TYPE(float);
COMPLEX_TYPE(double);
COMPLEX_TYPE(long_double);
COMPLEX_TYPE(interchange_float32);
COMPLEX_TYPE(interchange_float64);
COMPLEX_TYPE(interchange_float80);
COMPLEX_TYPE(extended_float32);
COMPLEX_TYPE(extended_float64);
#undef COMPLEX_TYPE

#define IMAGINARY_TYPE(id) const struct kefir_ast_type *kefir_ast_type_imaginary_##id(void)
IMAGINARY_TYPE(float);
IMAGINARY_TYPE(double);
IMAGINARY_TYPE(long_double);
IMAGINARY_TYPE(interchange_float32);
IMAGINARY_TYPE(interchange_float64);
IMAGINARY_TYPE(interchange_float80);
IMAGINARY_TYPE(extended_float32);
IMAGINARY_TYPE(extended_float64);
#undef IMAGINARY_TYPE

const struct kefir_ast_type *kefir_ast_type_signed_bitprecise(struct kefir_mem *, struct kefir_ast_type_bundle *,
                                                              kefir_size_t);
const struct kefir_ast_type *kefir_ast_type_unsigned_bitprecise(struct kefir_mem *, struct kefir_ast_type_bundle *,
                                                                kefir_size_t);

#define KEFIR_AST_TYPE_IS_CHARACTER(base)                                                             \
    ((base)->tag == KEFIR_AST_TYPE_SCALAR_CHAR || (base)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR || \
     (base)->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR)
#define KEFIR_INTERNAL_AST_TYPE_IS_SIGNED_INTEGER(base)                                                       \
    ((base)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_CHAR || (base)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_SHORT || \
     (base)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_INT || (base)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_LONG ||   \
     (base)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_LONG_LONG || (base)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE)
#define KEFIR_INTERNAL_AST_TYPE_IS_UNSIGNED_INTEGER(base)                                                             \
    ((base)->tag == KEFIR_AST_TYPE_SCALAR_BOOL || (base)->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_CHAR ||               \
     (base)->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_SHORT || (base)->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_INT ||      \
     (base)->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG || (base)->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_LONG_LONG || \
     (base)->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE)
#define KEFIR_AST_TYPE_IS_BIT_PRECISE_INTEGRAL_TYPE(base)       \
    ((base)->tag == KEFIR_AST_TYPE_SCALAR_SIGNED_BIT_PRECISE || \
     (base)->tag == KEFIR_AST_TYPE_SCALAR_UNSIGNED_BIT_PRECISE)
#define KEFIR_AST_TYPE_IS_NONENUM_INTEGRAL_TYPE(base)                                        \
    (KEFIR_AST_TYPE_IS_CHARACTER(base) || KEFIR_INTERNAL_AST_TYPE_IS_SIGNED_INTEGER(base) || \
     KEFIR_INTERNAL_AST_TYPE_IS_UNSIGNED_INTEGER(base))
#define KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(base) \
    (KEFIR_AST_TYPE_IS_NONENUM_INTEGRAL_TYPE(base) || (base)->tag == KEFIR_AST_TYPE_ENUMERATION)
#define KEFIR_AST_TYPE_IS_STANDARD_FLOATING_POINT(base)                                               \
    ((base)->tag == KEFIR_AST_TYPE_SCALAR_FLOAT || (base)->tag == KEFIR_AST_TYPE_SCALAR_DOUBLE || \
     (base)->tag == KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE)
#define KEFIR_AST_TYPE_IS_DECIMAL_FLOATING_POINT(base)                                               \
    ((base)->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL32 || (base)->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL64 || \
     (base)->tag == KEFIR_AST_TYPE_SCALAR_DECIMAL128 || (base)->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_DECIMAL64)
#define KEFIR_AST_TYPE_IS_BINARY_FLOATING_POINT(base)                                               \
    ((base)->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT32 || (base)->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT64 || (base)->tag == KEFIR_AST_TYPE_SCALAR_INTERCHANGE_FLOAT80 || \
    (base)->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT32 || (base)->tag == KEFIR_AST_TYPE_SCALAR_EXTENDED_FLOAT64)
#define KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(base)                                               \
    (KEFIR_AST_TYPE_IS_STANDARD_FLOATING_POINT(base) || KEFIR_AST_TYPE_IS_DECIMAL_FLOATING_POINT(base) || KEFIR_AST_TYPE_IS_BINARY_FLOATING_POINT(base))
#define KEFIR_AST_TYPE_IS_LONG_DOUBLE(base) ((base)->tag == KEFIR_AST_TYPE_SCALAR_LONG_DOUBLE)
#define KEFIR_AST_TYPE_IS_REAL_TYPE(base) \
    (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(base) || KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(base))
#define KEFIR_AST_TYPE_IS_COMPLEX_TYPE(base)                                                        \
    ((base)->tag == KEFIR_AST_TYPE_COMPLEX_FLOATING_POINT)
#define KEFIR_AST_TYPE_IS_IMAGINARY_TYPE(base)                                                        \
    ((base)->tag == KEFIR_AST_TYPE_IMAGINARY_FLOATING_POINT)
#define KEFIR_AST_TYPE_IS_FLOATING_POINT(base) \
    (KEFIR_AST_TYPE_IS_REAL_FLOATING_POINT(base) || KEFIR_AST_TYPE_IS_COMPLEX_TYPE(base) || KEFIR_AST_TYPE_IS_IMAGINARY_TYPE(base))
#define KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(base) \
    (KEFIR_AST_TYPE_IS_INTEGRAL_TYPE(base) || KEFIR_AST_TYPE_IS_FLOATING_POINT(base))
#define KEFIR_AST_TYPE_IS_SCALAR_TYPE(base)                                                     \
    (KEFIR_AST_TYPE_IS_ARITHMETIC_TYPE(base) || (base)->tag == KEFIR_AST_TYPE_SCALAR_POINTER || \
     (base)->tag == KEFIR_AST_TYPE_SCALAR_NULL_POINTER)
#define KEFIR_AST_TYPE_IS_AGGREGATE_TYPE(base) \
    (!KEFIR_AST_TYPE_IS_SCALAR_TYPE(base) && (base)->tag != KEFIR_AST_TYPE_VOID)

const struct kefir_ast_type *kefir_ast_type_flip_integer_singedness(const struct kefir_ast_type_traits *,
                                                                    const struct kefir_ast_type *);
const struct kefir_ast_type *kefir_ast_type_corresponding_real_type(const struct kefir_ast_type *);
const struct kefir_ast_type *kefir_ast_type_corresponding_complex_type(const struct kefir_ast_type *);
const struct kefir_ast_type *kefir_ast_type_corresponding_imaginary_type(const struct kefir_ast_type *);

kefir_result_t kefir_ast_type_is_signed(const struct kefir_ast_type_traits *, const struct kefir_ast_type *,
                                        kefir_bool_t *);

#endif

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

#ifndef KEFIR_AST_CONSTANT_EXPRESSION_H_
#define KEFIR_AST_CONSTANT_EXPRESSION_H_

#include "kefir/core/mem.h"
#include "kefir/ast/base.h"
#include "kefir/ast/constants.h"
#include "kefir/core/source_location.h"
#include "kefir/util/dfp.h"

typedef enum kefir_ast_constant_expression_class {
    KEFIR_AST_CONSTANT_EXPRESSION_CLASS_NONE,
    KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER,
    KEFIR_AST_CONSTANT_EXPRESSION_CLASS_FLOAT,
    KEFIR_AST_CONSTANT_EXPRESSION_CLASS_DECIMAL,
    KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPLEX_FLOAT,
    KEFIR_AST_CONSTANT_EXPRESSION_CLASS_ADDRESS,
    KEFIR_AST_CONSTANT_EXPRESSION_CLASS_COMPOUND
} kefir_ast_constant_expression_class_t;

typedef enum kefir_ast_constant_expression_pointer_base_type {
    KEFIR_AST_CONSTANT_EXPRESSION_POINTER_IDENTIFER,
    KEFIR_AST_CONSTANT_EXPRESSION_POINTER_INTEGER,
    KEFIR_AST_CONSTANT_EXPRESSION_POINTER_LITERAL
} kefir_ast_constant_expression_pointer_base_type_t;

typedef kefir_long_double_t kefir_ast_constant_expression_float_t;
typedef kefir_dfp_decimal128_t kefir_ast_constant_expression_decimal_t;

#define KEFIR_AST_CONSTANT_EXPRESSION_INT_MIN KEFIR_INT64_MIN
#define KEFIR_AST_CONSTANT_EXPRESSION_INT_MAX KEFIR_INT64_MAX

typedef struct kefir_ast_constant_expression_pointer {
    kefir_ast_constant_expression_pointer_base_type_t type;
    union {
        const char *literal;
        kefir_size_t integral;
        const struct kefir_ast_string_literal_data *string;
    } base;

    const struct kefir_ast_scoped_identifier *scoped_id;
    kefir_int64_t offset;
} kefir_ast_constant_expression_pointer_t;

typedef struct kefir_ast_constant_expression_value {
    kefir_ast_constant_expression_class_t klass;
    union {
        struct {
            union {
                kefir_ast_constant_expression_int_t integer;
                kefir_ast_constant_expression_uint_t uinteger;
            };
            struct kefir_bigint *bitprecise;
        };
        kefir_uint64_t floating_point[2];
        kefir_ast_constant_expression_decimal_t decimal;
        struct {
            kefir_uint64_t real[2];
            kefir_uint64_t imaginary[2];
        } complex_floating_point;
        struct kefir_ast_constant_expression_pointer pointer;
        struct {
            const struct kefir_ast_type *type;
            const struct kefir_ast_initializer *initializer;
        } compound;
    };
} kefir_ast_constant_expression_value_t;

kefir_ast_constant_expression_float_t kefir_ast_constant_expression_get_float(const kefir_uint64_t[2]);
void kefir_ast_constant_expression_set_float(kefir_uint64_t[2], kefir_ast_constant_expression_float_t);

#define KEFIR_AST_CONSTANT_EXPRESSION_GET_FLOAT(_value) \
    (kefir_ast_constant_expression_get_float((_value)->floating_point))

#define KEFIR_AST_CONSTANT_EXPRESSION_GET_COMPLEX_REAL(_value) \
    (kefir_ast_constant_expression_get_float((_value)->complex_floating_point.real))

#define KEFIR_AST_CONSTANT_EXPRESSION_GET_COMPLEX_IMAGINARY(_value) \
    (kefir_ast_constant_expression_get_float((_value)->complex_floating_point.imaginary))

#define KEFIR_AST_CONSTANT_EXPRESSION_SET_FLOAT(_value, _fp) \
    (kefir_ast_constant_expression_set_float((_value)->floating_point, (_fp)))

#define KEFIR_AST_CONSTANT_EXPRESSION_SET_COMPLEX_REAL(_value, _fp) \
    (kefir_ast_constant_expression_set_float((_value)->complex_floating_point.real, (_fp)))

#define KEFIR_AST_CONSTANT_EXPRESSION_SET_COMPLEX_IMAGINARY(_value, _fp) \
    (kefir_ast_constant_expression_set_float((_value)->complex_floating_point.imaginary, (_fp)))

#define KEFIR_AST_CONSTANT_EXPRESSION_INT_VALUE(_value)                                                  \
    ((struct kefir_ast_constant_expression_value) {.klass = KEFIR_AST_CONSTANT_EXPRESSION_CLASS_INTEGER, \
                                                   .integer = (_value)})

kefir_result_t kefir_ast_constant_expression_value_evaluate(struct kefir_mem *, const struct kefir_ast_context *,
                                                            const struct kefir_ast_node_base *,
                                                            struct kefir_ast_constant_expression_value *);

kefir_result_t kefir_ast_constant_expression_value_cast(struct kefir_mem *, const struct kefir_ast_context *,
                                                        struct kefir_ast_constant_expression_value *,
                                                        const struct kefir_ast_constant_expression_value *,
                                                        const struct kefir_ast_node_base *,
                                                        const struct kefir_ast_type *, const struct kefir_ast_type *);

kefir_result_t kefir_ast_constant_expression_value_to_boolean(const struct kefir_ast_constant_expression_value *,
                                                              kefir_bool_t *);

kefir_result_t kefir_ast_evaluate_comparison(struct kefir_mem *, const struct kefir_ast_context *,
                                             struct kefir_ast_node_base *, struct kefir_ast_node_base *, kefir_int_t *);

kefir_result_t kefir_ast_constant_expression_value_equal(const struct kefir_ast_constant_expression_value *,
                                                         const struct kefir_ast_constant_expression_value *,
                                                         kefir_bool_t *);

kefir_result_t kefir_ast_constant_expression_is_statically_known(const struct kefir_ast_constant_expression_value *,
                                                                 kefir_bool_t *);

kefir_result_t kefir_ast_constant_expression_evaluate_node(struct kefir_mem *, const struct kefir_ast_context *, struct kefir_ast_node_base *);

#endif

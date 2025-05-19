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

#include "kefir/ast/node.h"
#include "kefir/ast/node_internal.h"
#include "kefir/core/util.h"
#include "kefir/core/error.h"

NODE_VISIT_IMPL(ast_constant_visit, kefir_ast_constant, constant)

kefir_result_t ast_constant_free(struct kefir_mem *mem, struct kefir_ast_node_base *base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(base != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid AST node base"));
    ASSIGN_DECL_CAST(struct kefir_ast_constant *, node, base->self);
    KEFIR_FREE(mem, node);
    return KEFIR_OK;
}

const struct kefir_ast_node_class AST_CONSTANT_CLASS = {
    .type = KEFIR_AST_CONSTANT, .visit = ast_constant_visit, .free = ast_constant_free};

struct kefir_ast_constant *kefir_ast_new_constant_bool(struct kefir_mem *mem, kefir_bool_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_BOOL_CONSTANT;
    constant->value.boolean = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_char(struct kefir_mem *mem, kefir_int_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_CHAR_CONSTANT;
    constant->value.character = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_wide_char(struct kefir_mem *mem, kefir_wchar_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_WIDE_CHAR_CONSTANT;
    constant->value.wide_character = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_unicode16_char(struct kefir_mem *mem, kefir_char16_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_UNICODE16_CHAR_CONSTANT;
    constant->value.unicode16_character = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_unicode32_char(struct kefir_mem *mem, kefir_char32_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_UNICODE32_CHAR_CONSTANT;
    constant->value.unicode32_character = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_int(struct kefir_mem *mem, kefir_int64_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_INT_CONSTANT;
    constant->value.integer = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_uint(struct kefir_mem *mem, kefir_uint64_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_UINT_CONSTANT;
    constant->value.uinteger = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_long(struct kefir_mem *mem, kefir_int64_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_LONG_CONSTANT;
    constant->value.long_integer = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_ulong(struct kefir_mem *mem, kefir_uint64_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_ULONG_CONSTANT;
    constant->value.ulong_integer = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_long_long(struct kefir_mem *mem, kefir_int64_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_LONG_LONG_CONSTANT;
    constant->value.long_long = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_ulong_long(struct kefir_mem *mem, kefir_uint64_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_ULONG_LONG_CONSTANT;
    constant->value.ulong_long = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_bitprecise(struct kefir_mem *mem, kefir_int64_t value, kefir_size_t width) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_BITPRECISE_CONSTANT;
    constant->value.bitprecise.integer = value;
    constant->value.bitprecise.width = width;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_unsigned_bitprecise(struct kefir_mem *mem, kefir_uint64_t value, kefir_size_t width) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_UNSIGNED_BITPRECISE_CONSTANT;
    constant->value.bitprecise.uinteger = value;
    constant->value.bitprecise.width = width;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_float(struct kefir_mem *mem, kefir_float32_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_FLOAT_CONSTANT;
    constant->value.float32 = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_double(struct kefir_mem *mem, kefir_float64_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_DOUBLE_CONSTANT;
    constant->value.float64 = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_long_double(struct kefir_mem *mem, kefir_long_double_t value) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_LONG_DOUBLE_CONSTANT;
    constant->value.long_double = value;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_complex_float(struct kefir_mem *mem, kefir_float32_t real,
                                                                kefir_float32_t imaginary) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_COMPLEX_FLOAT_CONSTANT;
    constant->value.complex_float32.real = real;
    constant->value.complex_float32.imaginary = imaginary;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_complex_double(struct kefir_mem *mem, kefir_float64_t real,
                                                                 kefir_float64_t imaginary) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_COMPLEX_DOUBLE_CONSTANT;
    constant->value.complex_float64.real = real;
    constant->value.complex_float64.imaginary = imaginary;
    return constant;
}

struct kefir_ast_constant *kefir_ast_new_constant_complex_long_double(struct kefir_mem *mem, kefir_long_double_t real,
                                                                      kefir_long_double_t imaginary) {
    REQUIRE(mem != NULL, NULL);
    struct kefir_ast_constant *constant = KEFIR_MALLOC(mem, sizeof(struct kefir_ast_constant));
    REQUIRE(constant != NULL, NULL);
    constant->base.refcount = 1;
    constant->base.klass = &AST_CONSTANT_CLASS;
    constant->base.self = constant;
    kefir_result_t res = kefir_ast_node_properties_init(&constant->base.properties);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    res = kefir_source_location_empty(&constant->base.source_location);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, constant);
        return NULL;
    });
    constant->type = KEFIR_AST_COMPLEX_LONG_DOUBLE_CONSTANT;
    constant->value.complex_long_double.real = real;
    constant->value.complex_long_double.imaginary = imaginary;
    return constant;
}

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

#include "kefir/util/bigint/bigint.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <string.h>

#define __KEFIR_BIGINT_USE_BIGINT_IMPL__
#define __KEFIR_BIGINT_CHAR_BIT CHAR_BIT
#include "kefir/util/bigint/bigint_impl.h"

kefir_result_t kefir_bigint_init(struct kefir_bigint *bigint) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to big integer"));

    bigint->digits = NULL;
    bigint->bitwidth = 0;
    bigint->capacity = 0;
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_free(struct kefir_mem *mem, struct kefir_bigint *bigint) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    KEFIR_FREE(mem, bigint->digits);
    memset(bigint, 0, sizeof(struct kefir_bigint));
    return KEFIR_OK;
}

static kefir_result_t bigint_ensure_width(struct kefir_mem *mem, struct kefir_bigint *bigint, kefir_size_t width) {
    const kefir_size_t required_capacity = (width + CHAR_BIT - 1) / CHAR_BIT;
    if (bigint->capacity < required_capacity) {
        kefir_uint8_t *new_digits = KEFIR_REALLOC(mem, bigint->digits, sizeof(kefir_uint8_t) * required_capacity);
        REQUIRE(new_digits != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to ensure bigint capacity"));
        bigint->digits = new_digits;
        bigint->capacity = required_capacity;
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_set_value(struct kefir_mem *mem, struct kefir_bigint *bigint, kefir_int64_t value) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    if (value >= KEFIR_INT8_MIN && value <= KEFIR_INT8_MAX) {
        REQUIRE_OK(bigint_ensure_width(mem, bigint, 8));
    } else if (value >= KEFIR_INT16_MIN && value <= KEFIR_INT16_MAX) {
        REQUIRE_OK(bigint_ensure_width(mem, bigint, 16));
    } else if (value >= KEFIR_INT32_MIN && value <= KEFIR_INT32_MAX) {
        REQUIRE_OK(bigint_ensure_width(mem, bigint, 32));
    } else {
        REQUIRE_OK(bigint_ensure_width(mem, bigint, 64));
    }
    __KEFIR_BIGINT_WIDTH_T width = 0;
    __kefir_bigint_result_t res =
        __kefir_bigint_set_signed_integer(bigint->digits, bigint->capacity * CHAR_BIT, &width, value);
    UNUSED(res);
    bigint->bitwidth = width;
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_get_value(const struct kefir_bigint *bigint, kefir_int64_t *value_ptr) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(value_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to integer"));

    __KEFIR_BIGINT_SIGNED_VALUE_T value;
    __kefir_bigint_result_t res =
        __kefir_bigint_get_signed_value(bigint->digits, (__KEFIR_BIGINT_WIDTH_T) bigint->bitwidth, &value);
    UNUSED(res);
    *value_ptr = value;
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_cast_signed(struct kefir_mem *mem, struct kefir_bigint *bigint, kefir_size_t width) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    REQUIRE_OK(bigint_ensure_width(mem, bigint, width));
    __kefir_bigint_result_t res =
        __kefir_bigint_cast_signed(bigint->digits, bigint->bitwidth, (__KEFIR_BIGINT_WIDTH_T) width);
    UNUSED(res);
    bigint->bitwidth = width;
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_add(struct kefir_bigint *lhs_bigint, const struct kefir_bigint *rhs_bigint) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth == rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    __kefir_bigint_result_t res =
        __kefir_bigint_add(lhs_bigint->digits, rhs_bigint->digits, (__KEFIR_BIGINT_WIDTH_T) lhs_bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_invert(struct kefir_bigint *bigint) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res = __kefir_bigint_invert(bigint->digits, (__KEFIR_BIGINT_WIDTH_T) bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_and(struct kefir_bigint *lhs_bigint, const struct kefir_bigint *rhs_bigint) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth == rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    __kefir_bigint_result_t res =
        __kefir_bigint_and(lhs_bigint->digits, rhs_bigint->digits, (__KEFIR_BIGINT_WIDTH_T) lhs_bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_or(struct kefir_bigint *lhs_bigint, const struct kefir_bigint *rhs_bigint) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth == rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    __kefir_bigint_result_t res =
        __kefir_bigint_or(lhs_bigint->digits, rhs_bigint->digits, (__KEFIR_BIGINT_WIDTH_T) lhs_bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_xor(struct kefir_bigint *lhs_bigint, const struct kefir_bigint *rhs_bigint) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth == rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    __kefir_bigint_result_t res =
        __kefir_bigint_xor(lhs_bigint->digits, rhs_bigint->digits, (__KEFIR_BIGINT_WIDTH_T) lhs_bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_negate(struct kefir_bigint *bigint) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res = __kefir_bigint_negate(bigint->digits, (__KEFIR_BIGINT_WIDTH_T) bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_subtract(struct kefir_bigint *lhs_bigint, const struct kefir_bigint *rhs_bigint) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth == rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    __kefir_bigint_result_t res =
        __kefir_bigint_subtract(lhs_bigint->digits, rhs_bigint->digits, (__KEFIR_BIGINT_WIDTH_T) lhs_bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_left_shift(struct kefir_bigint *lhs_bigint, kefir_size_t shift) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res =
        __kefir_bigint_left_shift(lhs_bigint->digits, (__KEFIR_BIGINT_WIDTH_T) shift, lhs_bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

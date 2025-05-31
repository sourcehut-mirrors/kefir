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

#include "kefir/util/bigint.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"
#include <math.h>
#include <float.h>
#include <string.h>

#define __KEFIR_BIGINT_USE_BIGINT_IMPL__
#define __KEFIR_BIGINT_CHAR_BIT CHAR_BIT
#define __KEFIR_BIGINT_FLT_MANT_DIG FLT_MANT_DIG
#define __KEFIR_BIGINT_DBL_MANT_DIG DBL_MANT_DIG
#define __KEFIR_BIGINT_LDBL_MANT_DIG LDBL_MANT_DIG
#include "kefir_bigint/bigint.h"

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

kefir_result_t kefir_bigint_copy(struct kefir_bigint *dest, const struct kefir_bigint *src) {
    REQUIRE(dest != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination big integer"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source big integer"));
    const kefir_size_t required_capacity = (src->bitwidth + CHAR_BIT - 1) / CHAR_BIT;
    REQUIRE(dest->capacity >= required_capacity,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_SPACE, "Destination big integer lacks capacity for copy"));

    (void) __kefir_bigint_copy(dest->digits, src->digits, src->bitwidth);
    dest->bitwidth = src->bitwidth;

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

kefir_result_t kefir_bigint_copy_resize(struct kefir_mem *mem, struct kefir_bigint *dest,
                                        const struct kefir_bigint *src) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(dest != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination big integer"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source big integer"));

    REQUIRE_OK(bigint_ensure_width(mem, dest, src->bitwidth));
    (void) __kefir_bigint_copy(dest->digits, src->digits, src->bitwidth);
    dest->bitwidth = src->bitwidth;

    return KEFIR_OK;
}

kefir_result_t kefir_bigint_move(struct kefir_bigint *dest, struct kefir_bigint *src) {
    REQUIRE(dest != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid destination big integer"));
    REQUIRE(src != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid source big integer"));

    memcpy(dest, src, sizeof(struct kefir_bigint));
    memset(src, 0, sizeof(struct kefir_bigint));
    return KEFIR_OK;
}

kefir_size_t kefir_bigint_min_native_signed_width(kefir_int64_t value) {
    return __kefir_bigint_native_signed_width(value);
}

kefir_size_t kefir_bigint_min_native_unsigned_width(kefir_uint64_t value) {
    return __kefir_bigint_native_unsigned_width(value);
}

kefir_size_t kefir_bigint_min_unsigned_width(const struct kefir_bigint *value) {
    REQUIRE(value != NULL, 0);

    return __kefir_bigint_get_min_unsigned_width(value->digits, value->bitwidth);
}

kefir_result_t kefir_bigint_resize_nocast(struct kefir_mem *mem, struct kefir_bigint *bigint, kefir_size_t width) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(width > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-zero big integer width"));

    REQUIRE_OK(bigint_ensure_width(mem, bigint, width));
    bigint->bitwidth = width;
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_resize_cast_signed(struct kefir_mem *mem, struct kefir_bigint *bigint, kefir_size_t width) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    if (width > bigint->bitwidth) {
        const kefir_size_t old_width = bigint->bitwidth;
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, bigint, width));
        REQUIRE_OK(kefir_bigint_cast_signed(bigint, old_width, bigint->bitwidth));
    } else if (width != bigint->bitwidth) {
        REQUIRE_OK(kefir_bigint_cast_signed(bigint, bigint->bitwidth, width));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, bigint, width));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_resize_cast_unsigned(struct kefir_mem *mem, struct kefir_bigint *bigint,
                                                 kefir_size_t width) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    if (width > bigint->bitwidth) {
        const kefir_size_t old_width = bigint->bitwidth;
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, bigint, width));
        REQUIRE_OK(kefir_bigint_cast_unsigned(bigint, old_width, bigint->bitwidth));
    } else if (width != bigint->bitwidth) {
        REQUIRE_OK(kefir_bigint_cast_unsigned(bigint, bigint->bitwidth, width));
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, bigint, width));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_set_signed_value(struct kefir_bigint *bigint, kefir_int64_t value) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res = __kefir_bigint_set_signed_integer(bigint->digits, bigint->bitwidth, value);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_set_unsigned_value(struct kefir_bigint *bigint, kefir_uint64_t value) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res = __kefir_bigint_set_unsigned_integer(bigint->digits, bigint->bitwidth, value);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_get_signed(const struct kefir_bigint *bigint, kefir_int64_t *value_ptr) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(value_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to integer"));

    __KEFIR_BIGINT_SIGNED_VALUE_T value;
    __kefir_bigint_result_t res =
        __kefir_bigint_get_signed_value(bigint->digits, (__KEFIR_BIGINT_WIDTH_T) bigint->bitwidth, &value);
    UNUSED(res);
    *value_ptr = value;
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_get_unsigned(const struct kefir_bigint *bigint, kefir_uint64_t *value_ptr) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(value_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to integer"));

    __KEFIR_BIGINT_UNSIGNED_VALUE_T value;
    __kefir_bigint_result_t res =
        __kefir_bigint_get_unsigned_value(bigint->digits, (__KEFIR_BIGINT_WIDTH_T) bigint->bitwidth, &value);
    UNUSED(res);
    *value_ptr = value;
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_get_bits(const struct kefir_bigint *bigint, kefir_size_t offset, kefir_size_t length,
                                     kefir_uint64_t *value_ptr) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(value_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to integer"));

    *value_ptr = __kefir_bigint_get_bits(bigint->digits, offset, length, bigint->bitwidth);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_cast_signed(struct kefir_bigint *bigint, kefir_size_t from_width, kefir_size_t to_width) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(from_width <= bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided bitwidth exceeds big integer bitwidth"));
    REQUIRE(to_width <= bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided bitwidth exceeds big integer bitwidth"));

    __kefir_bigint_result_t res = __kefir_bigint_cast_signed(bigint->digits, (__KEFIR_BIGINT_WIDTH_T) from_width,
                                                             (__KEFIR_BIGINT_WIDTH_T) to_width);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_cast_unsigned(struct kefir_bigint *bigint, kefir_size_t from_width, kefir_size_t to_width) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(from_width <= bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided bitwidth exceeds big integer bitwidth"));
    REQUIRE(to_width <= bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_OUT_OF_BOUNDS, "Provided bitwidth exceeds big integer bitwidth"));

    __kefir_bigint_result_t res = __kefir_bigint_cast_unsigned(bigint->digits, (__KEFIR_BIGINT_WIDTH_T) from_width,
                                                               (__KEFIR_BIGINT_WIDTH_T) to_width);
    UNUSED(res);
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

kefir_result_t kefir_bigint_right_shift(struct kefir_bigint *lhs_bigint, kefir_size_t shift) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res =
        __kefir_bigint_right_shift(lhs_bigint->digits, (__KEFIR_BIGINT_WIDTH_T) shift, lhs_bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_arithmetic_right_shift(struct kefir_bigint *lhs_bigint, kefir_size_t shift) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res =
        __kefir_bigint_arithmetic_right_shift(lhs_bigint->digits, (__KEFIR_BIGINT_WIDTH_T) shift, lhs_bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_multiply(struct kefir_bigint *result_bigint, const struct kefir_bigint *lhs_bigint,
                                              const struct kefir_bigint *rhs_bigint, struct kefir_bigint *tmp_bigint) {
    REQUIRE(result_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(tmp_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth == rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));
    REQUIRE(tmp_bigint->bitwidth >= result_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Temporary big integer shall be at least as large as result"));

    __kefir_bigint_result_t res =
        __kefir_bigint_unsigned_multiply(result_bigint->digits, tmp_bigint->digits, lhs_bigint->digits,
                                         rhs_bigint->digits, result_bigint->bitwidth, lhs_bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_signed_multiply(struct kefir_bigint *result_bigint, struct kefir_bigint *lhs_bigint,
                                            const struct kefir_bigint *rhs_bigint,
                                            struct kefir_bigint *accumulator_bigint) {
    REQUIRE(result_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(accumulator_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth == rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));
    REQUIRE(accumulator_bigint->bitwidth >= lhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Temporary big integer shall be at least as large as operand"));

    __kefir_bigint_result_t res =
        __kefir_bigint_signed_multiply(result_bigint->digits, accumulator_bigint->digits, lhs_bigint->digits,
                                       rhs_bigint->digits, result_bigint->bitwidth, lhs_bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_divide(struct kefir_bigint *lhs_bigint, struct kefir_bigint *remainder_bigint,
                                            const struct kefir_bigint *rhs_bigint) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(remainder_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth >= rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));
    REQUIRE(lhs_bigint->bitwidth == remainder_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    __kefir_bigint_result_t res = __kefir_bigint_unsigned_divide(
        lhs_bigint->digits, remainder_bigint->digits, rhs_bigint->digits, lhs_bigint->bitwidth, rhs_bigint->bitwidth);
    if (res == __KEFIR_BIGINT_DIVISION_BY_ZERO) {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Division by zero occured");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_signed_divide(struct kefir_bigint *lhs_bigint, struct kefir_bigint *remainder_bigint,
                                          struct kefir_bigint *rhs_bigint) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(remainder_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth >= rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));
    REQUIRE(lhs_bigint->bitwidth == remainder_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    __kefir_bigint_result_t res = __kefir_bigint_signed_divide(
        lhs_bigint->digits, remainder_bigint->digits, rhs_bigint->digits, lhs_bigint->bitwidth, rhs_bigint->bitwidth);
    if (res == __KEFIR_BIGINT_DIVISION_BY_ZERO) {
        return KEFIR_SET_ERROR(KEFIR_INVALID_REQUEST, "Division by zero occured");
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_compare(const struct kefir_bigint *lhs_bigint,
                                             const struct kefir_bigint *rhs_bigint, kefir_int_t *comparison_ptr) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth == rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));
    REQUIRE(comparison_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to comparison result"));

    *comparison_ptr =
        (kefir_int_t) __kefir_bigint_unsigned_compare(lhs_bigint->digits, rhs_bigint->digits, lhs_bigint->bitwidth);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_signed_compare(const struct kefir_bigint *lhs_bigint, const struct kefir_bigint *rhs_bigint,
                                           kefir_int_t *comparison_ptr) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth == rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));
    REQUIRE(comparison_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to comparison result"));

    *comparison_ptr =
        (kefir_int_t) __kefir_bigint_signed_compare(lhs_bigint->digits, rhs_bigint->digits, lhs_bigint->bitwidth);
    return KEFIR_OK;
}

static kefir_result_t parse10_impl(struct kefir_bigint *bigint, const char *input, kefir_size_t input_length,
                                   struct kefir_bigint *tmp_bigints) {
    REQUIRE_OK(kefir_bigint_set_unsigned_value(bigint, 0));
    for (kefir_size_t i = 0; i < input_length; i++) {
        kefir_uint8_t digit = 0;
        if (input[i] >= '0' && input[i] <= '9') {
            digit = input[i] - '0';
        } else if (input[i] == '\0') {
            break;
        } else {
            return KEFIR_SET_ERRORF(KEFIR_INVALID_STATE, "Unexpected character in decimal big integer: '%c' (%d)",
                                    input[i], input[i]);
        }

        REQUIRE_OK(kefir_bigint_copy(&tmp_bigints[0], bigint));
        REQUIRE_OK(kefir_bigint_set_unsigned_value(&tmp_bigints[1], 10));
        REQUIRE_OK(kefir_bigint_unsigned_multiply(bigint, &tmp_bigints[0], &tmp_bigints[1], &tmp_bigints[2]));
        REQUIRE_OK(kefir_bigint_set_unsigned_value(&tmp_bigints[0], digit));
        REQUIRE_OK(kefir_bigint_add(bigint, &tmp_bigints[0]));
    }

    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_parse10_into(struct kefir_mem *mem, struct kefir_bigint *bigint, const char *input,
                                                  kefir_size_t input_length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(input != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid input string"));

    struct kefir_bigint tmp_bigints[3];
    REQUIRE_OK(kefir_bigint_init(&tmp_bigints[0]));
    REQUIRE_OK(kefir_bigint_init(&tmp_bigints[1]));
    REQUIRE_OK(kefir_bigint_init(&tmp_bigints[2]));
    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, kefir_bigint_resize_cast_unsigned(mem, &tmp_bigints[0], bigint->bitwidth));
    REQUIRE_CHAIN(&res, kefir_bigint_resize_cast_unsigned(mem, &tmp_bigints[1], bigint->bitwidth));
    REQUIRE_CHAIN(&res, kefir_bigint_resize_cast_unsigned(mem, &tmp_bigints[2], bigint->bitwidth));
    REQUIRE_CHAIN(&res, parse10_impl(bigint, input, input_length, tmp_bigints));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bigint_free(mem, &tmp_bigints[0]);
        kefir_bigint_free(mem, &tmp_bigints[1]);
        kefir_bigint_free(mem, &tmp_bigints[2]);
        return res;
    });
    res = kefir_bigint_free(mem, &tmp_bigints[0]);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bigint_free(mem, &tmp_bigints[1]);
        kefir_bigint_free(mem, &tmp_bigints[2]);
        return res;
    });
    res = kefir_bigint_free(mem, &tmp_bigints[1]);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bigint_free(mem, &tmp_bigints[2]);
        return res;
    });
    REQUIRE_OK(kefir_bigint_free(mem, &tmp_bigints[2]));

    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_parse10(struct kefir_mem *mem, struct kefir_bigint *bigint, const char *input,
                                             kefir_size_t input_length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(input != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid input string"));

    const kefir_size_t input_strlen = strlen(input);
    if (input_strlen < input_length) {
        input_length = input_strlen;
    }

    const kefir_size_t num_of_bits = 1 + (kefir_size_t) ceil(input_length * 3.322 /* log2(10) */);
    REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, bigint, num_of_bits));
    REQUIRE_OK(kefir_bigint_unsigned_parse10_into(mem, bigint, input, input_length));

    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_parse16_into(struct kefir_mem *mem, struct kefir_bigint *bigint, const char *input,
                                                  kefir_size_t input_length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(input != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid input string"));

    REQUIRE_OK(kefir_bigint_set_unsigned_value(bigint, 0));
    for (kefir_size_t i = 0; i < input_length; i++) {
        const kefir_size_t index = input_length - i - 1;
        kefir_uint8_t digit = 0;
        switch (input[i]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                digit = input[i] - '0';
                break;

            case 'a':
            case 'A':
                digit = 0xa;
                break;

            case 'b':
            case 'B':
                digit = 0xb;
                break;

            case 'c':
            case 'C':
                digit = 0xc;
                break;

            case 'd':
            case 'D':
                digit = 0xd;
                break;

            case 'e':
            case 'E':
                digit = 0xe;
                break;

            case 'f':
            case 'F':
                digit = 0xf;
                break;

            case '\0':
                return KEFIR_OK;

            default:
                return KEFIR_SET_ERRORF(KEFIR_INVALID_STATE,
                                        "Unexpected character in hexadecimal big integer: '%c' (%d)", input[index],
                                        input[index]);
        }

        (void) __kefir_bigint_set_bits(bigint->digits, digit, index * 4, 4, bigint->bitwidth);
    }

    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_parse16(struct kefir_mem *mem, struct kefir_bigint *bigint, const char *input,
                                             kefir_size_t input_length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(input != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid input string"));

    const kefir_size_t input_strlen = strlen(input);
    if (input_strlen < input_length) {
        input_length = input_strlen;
    }

    const kefir_size_t num_of_bits = input_length * 4;
    REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, bigint, num_of_bits));
    REQUIRE_OK(kefir_bigint_unsigned_parse16_into(mem, bigint, input, input_length));

    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_parse8_into(struct kefir_mem *mem, struct kefir_bigint *bigint, const char *input,
                                                 kefir_size_t input_length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(input != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid input string"));

    REQUIRE_OK(kefir_bigint_set_unsigned_value(bigint, 0));
    for (kefir_size_t i = 0; i < input_length; i++) {
        const kefir_size_t index = input_length - i - 1;
        kefir_uint8_t digit = 0;
        switch (input[i]) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                digit = input[i] - '0';
                break;

            case '\0':
                return KEFIR_OK;

            default:
                return KEFIR_SET_ERRORF(KEFIR_INVALID_STATE, "Unexpected character in octal big integer: '%c' (%d)",
                                        input[index], input[index]);
        }

        (void) __kefir_bigint_set_bits(bigint->digits, digit, index * 3, 3, bigint->bitwidth);
    }

    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_parse8(struct kefir_mem *mem, struct kefir_bigint *bigint, const char *input,
                                            kefir_size_t input_length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(input != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid input string"));

    const kefir_size_t input_strlen = strlen(input);
    if (input_strlen < input_length) {
        input_length = input_strlen;
    }

    const kefir_size_t num_of_bits = input_length * 3;
    REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, bigint, num_of_bits));
    REQUIRE_OK(kefir_bigint_unsigned_parse8_into(mem, bigint, input, input_length));

    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_parse2_into(struct kefir_mem *mem, struct kefir_bigint *bigint, const char *input,
                                                 kefir_size_t input_length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(input != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid input string"));

    REQUIRE_OK(kefir_bigint_set_unsigned_value(bigint, 0));
    for (kefir_size_t i = 0; i < input_length; i++) {
        const kefir_size_t index = input_length - i - 1;
        kefir_uint8_t digit = 0;
        switch (input[i]) {
            case '0':
            case '1':
                digit = input[i] - '0';
                break;

            case '\0':
                return KEFIR_OK;

            default:
                return KEFIR_SET_ERRORF(KEFIR_INVALID_STATE, "Unexpected character in binary big integer: '%c' (%d)",
                                        input[index], input[index]);
        }

        (void) __kefir_bigint_set_bits(bigint->digits, digit, index, 1, bigint->bitwidth);
    }

    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_parse2(struct kefir_mem *mem, struct kefir_bigint *bigint, const char *input,
                                            kefir_size_t input_length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(input != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid input string"));

    const kefir_size_t input_strlen = strlen(input);
    if (input_strlen < input_length) {
        input_length = input_strlen;
    }

    const kefir_size_t num_of_bits = input_length;
    REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, bigint, num_of_bits));
    REQUIRE_OK(kefir_bigint_unsigned_parse2_into(mem, bigint, input, input_length));

    return KEFIR_OK;
}

kefir_result_t kefir_bigint_signed_parse(struct kefir_mem *mem, struct kefir_bigint *bigint, const char *input,
                                         kefir_size_t input_length, kefir_uint_t base) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(input != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid input string"));

    const kefir_size_t input_strlen = strlen(input);
    input_length = MIN(input_length, input_strlen);

    kefir_bool_t negate = false;
    if (input_length > 0 && input[0] == '-') {
        negate = true;
        input_length--;
        input++;
    }

    switch (base) {
        case 2:
            REQUIRE_OK(kefir_bigint_unsigned_parse2(mem, bigint, input, input_length));
            break;

        case 8:
            REQUIRE_OK(kefir_bigint_unsigned_parse8(mem, bigint, input, input_length));
            break;

        case 10:
            REQUIRE_OK(kefir_bigint_unsigned_parse10(mem, bigint, input, input_length));
            break;

        case 16:
            REQUIRE_OK(kefir_bigint_unsigned_parse16(mem, bigint, input, input_length));
            break;

        default:
            return KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Unexpected big integer base");
    }

    if (__kefir_bigint_get_sign(bigint->digits, bigint->bitwidth)) {
        REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, bigint, bigint->bitwidth + 1));
    }
    if (negate) {
        REQUIRE_OK(kefir_bigint_negate(bigint));
    }

    return KEFIR_OK;
}

static kefir_result_t unsigned_format10_into_impl(struct kefir_mem *mem, struct kefir_bigint *bigint, char *output,
                                                  kefir_size_t output_length, struct kefir_bigint *base10,
                                                  struct kefir_bigint *digit) {
    const kefir_size_t old_bitwidth = bigint->bitwidth;
    if (bigint->bitwidth < CHAR_BIT) {
        REQUIRE_OK(kefir_bigint_resize_cast_unsigned(mem, bigint, CHAR_BIT));
    }
    REQUIRE_OK(kefir_bigint_resize_nocast(mem, base10, bigint->bitwidth));
    REQUIRE_OK(kefir_bigint_resize_nocast(mem, digit, bigint->bitwidth));
    REQUIRE_OK(kefir_bigint_set_unsigned_value(base10, 10));

    kefir_size_t i = 0;
    for (; i < output_length - 1; i++) {
        if (__kefir_bigint_is_zero(bigint->digits, bigint->bitwidth)) {
            if (i == 0) {
                output[i++] = '0';
            }
            break;
        }

        kefir_uint64_t digit_value;
        REQUIRE_OK(kefir_bigint_unsigned_divide(bigint, digit, base10));
        REQUIRE_OK(kefir_bigint_get_unsigned(digit, &digit_value));
        REQUIRE(digit_value <= 9, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected decimal digit value"));
        output[i] = '0' + digit_value;
    }

    for (kefir_size_t j = 0; j < i / 2; j++) {
        const char tmp = output[j];
        output[j] = output[i - j - 1];
        output[i - j - 1] = tmp;
    }
    output[i] = '\0';

    if (bigint->bitwidth != old_bitwidth) {
        REQUIRE_OK(kefir_bigint_resize_nocast(mem, bigint, old_bitwidth));
    }
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_format10_into(struct kefir_mem *mem, struct kefir_bigint *bigint, char *output,
                                                   kefir_size_t output_length) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid output string"));
    REQUIRE(output_length > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-empty output string"));

    struct kefir_bigint base10, digit;
    REQUIRE_OK(kefir_bigint_init(&base10));
    REQUIRE_OK(kefir_bigint_init(&digit));

    kefir_result_t res = KEFIR_OK;
    REQUIRE_CHAIN(&res, unsigned_format10_into_impl(mem, bigint, output, output_length, &base10, &digit));
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bigint_free(mem, &base10);
        kefir_bigint_free(mem, &digit);
        return res;
    });
    res = kefir_bigint_free(mem, &base10);
    REQUIRE_ELSE(res == KEFIR_OK, {
        kefir_bigint_free(mem, &digit);
        return res;
    });
    REQUIRE_OK(kefir_bigint_free(mem, &digit));

    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_format10(struct kefir_mem *mem, struct kefir_bigint *bigint, char **output_ptr,
                                              kefir_size_t *output_length_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(output_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to output string"));

    const kefir_size_t output_length = 1 + (kefir_size_t) ceil(bigint->bitwidth * 0.302 /* log10(2) */);
    char *output = KEFIR_MALLOC(mem, sizeof(char) * output_length);
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate formatted big integer"));
    kefir_result_t res = kefir_bigint_unsigned_format10_into(mem, bigint, output, output_length);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, output);
        return res;
    });

    *output_ptr = output;
    ASSIGN_PTR(output_length_ptr, output_length);
    return KEFIR_OK;
}

static kefir_result_t unsigned_format16_into_impl(const struct kefir_bigint *bigint, char *output,
                                                  kefir_size_t output_length) {
    kefir_size_t i = 0;
    for (; i < output_length - 1; i++) {
        if (__kefir_bigint_is_zero(bigint->digits, bigint->bitwidth)) {
            if (i == 0) {
                output[i++] = '0';
            }
            break;
        }

        kefir_uint64_t digit_value = __kefir_bigint_get_bits(bigint->digits, i * 4, 4, bigint->bitwidth);
        REQUIRE(digit_value <= 0xf, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected hexadecimal digit value"));
        switch (digit_value) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                output[i] = '0' + digit_value;
                break;

            case 0xa:
                output[i] = 'a';
                break;

            case 0xb:
                output[i] = 'b';
                break;

            case 0xc:
                output[i] = 'c';
                break;

            case 0xd:
                output[i] = 'd';
                break;

            case 0xe:
                output[i] = 'e';
                break;

            case 0xf:
                output[i] = 'f';
                break;
        }
    }

    for (; i > 1 && output[i - 1] == '0'; i--) {}

    for (kefir_size_t j = 0; j < i / 2; j++) {
        const char tmp = output[j];
        output[j] = output[i - j - 1];
        output[i - j - 1] = tmp;
    }
    output[i] = '\0';
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_format16_into(const struct kefir_bigint *bigint, char *output,
                                                   kefir_size_t output_length) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid output string"));
    REQUIRE(output_length > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-empty output string"));

    REQUIRE_OK(unsigned_format16_into_impl(bigint, output, output_length));
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_format16(struct kefir_mem *mem, struct kefir_bigint *bigint, char **output_ptr,
                                              kefir_size_t *output_length_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(output_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to output string"));

    const kefir_size_t output_length = 1 + (bigint->bitwidth + 3) / 4;
    char *output = KEFIR_MALLOC(mem, sizeof(char) * output_length);
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate formatted big integer"));
    kefir_result_t res = kefir_bigint_unsigned_format16_into(bigint, output, output_length);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, output);
        return res;
    });

    *output_ptr = output;
    ASSIGN_PTR(output_length_ptr, output_length);
    return KEFIR_OK;
}

static kefir_result_t unsigned_format8_into_impl(const struct kefir_bigint *bigint, char *output,
                                                 kefir_size_t output_length) {
    kefir_size_t i = 0;
    for (; i < output_length - 1; i++) {
        if (__kefir_bigint_is_zero(bigint->digits, bigint->bitwidth)) {
            if (i == 0) {
                output[i++] = '0';
            }
            break;
        }

        kefir_uint64_t digit_value = __kefir_bigint_get_bits(bigint->digits, i * 3, 3, bigint->bitwidth);
        REQUIRE(digit_value < 8, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected hexadecimal digit value"));
        output[i] = '0' + digit_value;
    }

    for (; i > 1 && output[i - 1] == '0'; i--) {}

    for (kefir_size_t j = 0; j < i / 2; j++) {
        const char tmp = output[j];
        output[j] = output[i - j - 1];
        output[i - j - 1] = tmp;
    }
    output[i] = '\0';
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_format8_into(const struct kefir_bigint *bigint, char *output,
                                                  kefir_size_t output_length) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid output string"));
    REQUIRE(output_length > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-empty output string"));

    REQUIRE_OK(unsigned_format8_into_impl(bigint, output, output_length));
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_format8(struct kefir_mem *mem, struct kefir_bigint *bigint, char **output_ptr,
                                             kefir_size_t *output_length_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(output_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to output string"));

    const kefir_size_t output_length = 1 + (bigint->bitwidth + 2) / 3;
    char *output = KEFIR_MALLOC(mem, sizeof(char) * output_length);
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate formatted big integer"));
    kefir_result_t res = kefir_bigint_unsigned_format8_into(bigint, output, output_length);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, output);
        return res;
    });

    *output_ptr = output;
    ASSIGN_PTR(output_length_ptr, output_length);
    return KEFIR_OK;
}

static kefir_result_t unsigned_format2_into_impl(const struct kefir_bigint *bigint, char *output,
                                                 kefir_size_t output_length) {
    kefir_size_t i = 0;
    for (; i < output_length - 1; i++) {
        if (__kefir_bigint_is_zero(bigint->digits, bigint->bitwidth)) {
            if (i == 0) {
                output[i++] = '0';
            }
            break;
        }

        kefir_uint64_t digit_value = __kefir_bigint_get_bits(bigint->digits, i, 1, bigint->bitwidth);
        REQUIRE(digit_value < 2, KEFIR_SET_ERROR(KEFIR_INVALID_STATE, "Unexpected hexadecimal digit value"));
        output[i] = '0' + digit_value;
    }

    for (; i > 1 && output[i - 1] == '0'; i--) {}

    for (kefir_size_t j = 0; j < i / 2; j++) {
        const char tmp = output[j];
        output[j] = output[i - j - 1];
        output[i - j - 1] = tmp;
    }
    output[i] = '\0';
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_format2_into(const struct kefir_bigint *bigint, char *output,
                                                  kefir_size_t output_length) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid output string"));
    REQUIRE(output_length > 0, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected non-empty output string"));

    REQUIRE_OK(unsigned_format2_into_impl(bigint, output, output_length));
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_format2(struct kefir_mem *mem, struct kefir_bigint *bigint, char **output_ptr,
                                             kefir_size_t *output_length_ptr) {
    REQUIRE(mem != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid memory allocator"));
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(output_ptr != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to output string"));

    const kefir_size_t output_length = 1 + bigint->bitwidth;
    char *output = KEFIR_MALLOC(mem, sizeof(char) * output_length);
    REQUIRE(output != NULL, KEFIR_SET_ERROR(KEFIR_MEMALLOC_FAILURE, "Failed to allocate formatted big integer"));
    kefir_result_t res = kefir_bigint_unsigned_format2_into(bigint, output, output_length);
    REQUIRE_ELSE(res == KEFIR_OK, {
        KEFIR_FREE(mem, output);
        return res;
    });

    *output_ptr = output;
    ASSIGN_PTR(output_length_ptr, output_length);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_signed_to_float(struct kefir_bigint *bigint, struct kefir_bigint *tmp_bigint,
                                            kefir_float32_t *value_ptr) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(tmp_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(value_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to floating-point value"));
    REQUIRE(bigint->bitwidth >= sizeof(kefir_float32_t) * CHAR_BIT,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Provided big integer is too narrow"));
    REQUIRE(bigint->bitwidth == tmp_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    *value_ptr = __kefir_bigint_signed_to_float(bigint->digits, tmp_bigint->digits, bigint->bitwidth);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_signed_to_double(struct kefir_bigint *bigint, struct kefir_bigint *tmp_bigint,
                                             kefir_float64_t *value_ptr) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(tmp_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(value_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to floating-point value"));
    REQUIRE(bigint->bitwidth >= sizeof(kefir_float64_t) * CHAR_BIT,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Provided big integer is too narrow"));
    REQUIRE(bigint->bitwidth == tmp_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    *value_ptr = __kefir_bigint_signed_to_double(bigint->digits, tmp_bigint->digits, bigint->bitwidth);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_signed_to_long_double(struct kefir_bigint *bigint, struct kefir_bigint *tmp_bigint,
                                                  kefir_long_double_t *value_ptr) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(tmp_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(value_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to floating-point value"));
    REQUIRE(bigint->bitwidth >= sizeof(kefir_long_double_t) * CHAR_BIT,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Provided big integer is too narrow"));
    REQUIRE(bigint->bitwidth == tmp_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    *value_ptr = __kefir_bigint_signed_to_long_double(bigint->digits, tmp_bigint->digits, bigint->bitwidth);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_to_float(struct kefir_bigint *bigint, struct kefir_bigint *tmp_bigint,
                                              kefir_float32_t *value_ptr) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(tmp_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(value_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to floating-point value"));
    REQUIRE(bigint->bitwidth >= sizeof(kefir_float32_t) * CHAR_BIT,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Provided big integer is too narrow"));
    REQUIRE(bigint->bitwidth == tmp_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    *value_ptr = __kefir_bigint_unsigned_to_float(bigint->digits, tmp_bigint->digits, bigint->bitwidth);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_to_double(struct kefir_bigint *bigint, struct kefir_bigint *tmp_bigint,
                                               kefir_float64_t *value_ptr) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(tmp_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(value_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to floating-point value"));
    REQUIRE(bigint->bitwidth >= sizeof(kefir_float64_t) * CHAR_BIT,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Provided big integer is too narrow"));
    REQUIRE(bigint->bitwidth == tmp_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    *value_ptr = __kefir_bigint_unsigned_to_double(bigint->digits, tmp_bigint->digits, bigint->bitwidth);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_to_long_double(struct kefir_bigint *bigint, struct kefir_bigint *tmp_bigint,
                                                    kefir_long_double_t *value_ptr) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(tmp_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(value_ptr != NULL,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid pointer to floating-point value"));
    REQUIRE(bigint->bitwidth >= sizeof(kefir_long_double_t) * CHAR_BIT,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Provided big integer is too narrow"));
    REQUIRE(bigint->bitwidth == tmp_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    *value_ptr = __kefir_bigint_unsigned_to_long_double(bigint->digits, tmp_bigint->digits, bigint->bitwidth);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_signed_from_float(struct kefir_bigint *bigint, kefir_float32_t value) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res = __kefir_bigint_signed_from_float(bigint->digits, value, bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_signed_from_double(struct kefir_bigint *bigint, kefir_float64_t value) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res = __kefir_bigint_signed_from_double(bigint->digits, value, bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_signed_from_long_double(struct kefir_bigint *bigint, kefir_long_double_t value) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res = __kefir_bigint_signed_from_long_double(bigint->digits, value, bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_from_float(struct kefir_bigint *bigint, kefir_float32_t value) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res = __kefir_bigint_unsigned_from_float(bigint->digits, value, bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_unsigned_from_double(struct kefir_bigint *bigint, kefir_float64_t value) {
    REQUIRE(bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));

    __kefir_bigint_result_t res = __kefir_bigint_unsigned_from_double(bigint->digits, value, bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

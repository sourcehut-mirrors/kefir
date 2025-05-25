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
#include <string.h>

#define __KEFIR_BIGINT_USE_BIGINT_IMPL__
#define __KEFIR_BIGINT_CHAR_BIT CHAR_BIT
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

kefir_size_t kefir_bigint_min_signed_width(kefir_int64_t value) {
    return __kefir_bigint_native_signed_width(value);
}

kefir_size_t kefir_bigint_min_unsigned_width(kefir_uint64_t value) {
    return __kefir_bigint_native_unsigned_width(value);
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
    REQUIRE(lhs_bigint->bitwidth == rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));
    REQUIRE(lhs_bigint->bitwidth == remainder_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    __kefir_bigint_result_t res = __kefir_bigint_unsigned_divide(lhs_bigint->digits, remainder_bigint->digits,
                                                                 rhs_bigint->digits, lhs_bigint->bitwidth);
    UNUSED(res);
    return KEFIR_OK;
}

kefir_result_t kefir_bigint_signed_divide(struct kefir_bigint *lhs_bigint, struct kefir_bigint *remainder_bigint,
                                          struct kefir_bigint *rhs_bigint) {
    REQUIRE(lhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(remainder_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(rhs_bigint != NULL, KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Expected valid big integer"));
    REQUIRE(lhs_bigint->bitwidth == rhs_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));
    REQUIRE(lhs_bigint->bitwidth == remainder_bigint->bitwidth,
            KEFIR_SET_ERROR(KEFIR_INVALID_PARAMETER, "Big integer width mismatch"));

    __kefir_bigint_result_t res = __kefir_bigint_signed_divide(lhs_bigint->digits, remainder_bigint->digits,
                                                               rhs_bigint->digits, lhs_bigint->bitwidth);
    UNUSED(res);
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

/*
    SPDX-License-Identifier: BSD-3-Clause

    Copyright 2020-2025 Jevgenijs Protopopovs

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its contributors
    may be used to endorse or promote products derived from this software without
    specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS
    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
    OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __KEFIR_BIGINT_DIVIDE_H__
#define __KEFIR_BIGINT_DIVIDE_H__

#include "kefir_bigint/base.h"

static __kefir_bigint_result_t __kefir_bigint_unsigned_divide(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                              __KEFIR_BIGINT_DIGIT_T *accumulator_digits,
                                                              const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                              __KEFIR_BIGINT_WIDTH_T lhs_width,
                                                              __KEFIR_BIGINT_WIDTH_T rhs_width) {
    (void) __kefir_bigint_zero(accumulator_digits, lhs_width);
    if (lhs_width == 0) {
        return __KEFIR_BIGINT_OK;
    }
    if (__kefir_bigint_is_zero(rhs_digits, rhs_width)) {
        return __KEFIR_BIGINT_DIVISION_BY_ZERO;
    }

    const __KEFIR_BIGINT_WIDTH_T msb_digit_index = (lhs_width - 1) / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T msb_digit_offset = lhs_width - 1 - msb_digit_index * __KEFIR_BIGINT_DIGIT_BIT;

    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < lhs_width; i++) {
        const __KEFIR_BIGINT_UINT_T lhs_msb = (lhs_digits[msb_digit_index] >> msb_digit_offset) & 1;
        (void) __kefir_bigint_left_shift(accumulator_digits, 1, lhs_width);
        (void) __kefir_bigint_left_shift(lhs_digits, 1, lhs_width);
        accumulator_digits[0] |= lhs_msb;

        (void) __kefir_bigint_subtract_zero_extend(accumulator_digits, rhs_digits, lhs_width, rhs_width);
        const __KEFIR_BIGINT_UINT_T acc_msb = (accumulator_digits[msb_digit_index] >> msb_digit_offset) & 1;
        if (acc_msb) {
            lhs_digits[0] &= ~(__KEFIR_BIGINT_DIGIT_T) 1;
            (void) __kefir_bigint_add_zero_extend(accumulator_digits, rhs_digits, lhs_width, rhs_width);
        } else {
            lhs_digits[0] |= 1;
        }
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_signed_divide(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                            __KEFIR_BIGINT_DIGIT_T *accumulator_digits,
                                                            __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                            __KEFIR_BIGINT_WIDTH_T lhs_width,
                                                            __KEFIR_BIGINT_WIDTH_T rhs_width) {
    const __KEFIR_BIGINT_UINT_T lhs_sign = __kefir_bigint_get_sign(lhs_digits, lhs_width);
    const __KEFIR_BIGINT_UINT_T rhs_sign = __kefir_bigint_get_sign(rhs_digits, rhs_width);
    if (__kefir_bigint_is_zero(rhs_digits, rhs_width)) {
        return __KEFIR_BIGINT_DIVISION_BY_ZERO;
    }

    if (lhs_sign) {
        (void) __kefir_bigint_negate(lhs_digits, lhs_width);
    }
    if (rhs_sign) {
        (void) __kefir_bigint_negate(rhs_digits, rhs_width);
    }

    (void) __kefir_bigint_unsigned_divide(lhs_digits, accumulator_digits, rhs_digits, lhs_width, rhs_width);
    if (lhs_sign ^ rhs_sign) {
        (void) __kefir_bigint_negate(lhs_digits, lhs_width);
    }
    if (lhs_sign) {
        (void) __kefir_bigint_negate(accumulator_digits, lhs_width);
    }

    return __KEFIR_BIGINT_OK;
}

#endif

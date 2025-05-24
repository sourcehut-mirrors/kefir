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

#ifndef __KEFIR_BIGINT_ADDSUB_H__
#define __KEFIR_BIGINT_ADDSUB_H__

#include "kefir_bigint/base.h"

static __kefir_bigint_result_t __kefir_bigint_util_add_digit_zero_extended(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                                           __KEFIR_BIGINT_DIGIT_T rhs_digit,
                                                                           __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    const __KEFIR_BIGINT_UINT_T rhs_extension = 0;

    __KEFIR_BIGINT_UINT_T carry = 0;
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_UINT_T lhs_digit = lhs_digits[i];
        const __KEFIR_BIGINT_UINT_T digit = i == 0 ? rhs_digit : rhs_extension;

        const __KEFIR_BIGINT_UINT_T digit_sum = carry + lhs_digit + digit;
        lhs_digits[i] = (__KEFIR_BIGINT_DIGIT_T) digit_sum;
        carry = digit_sum >> __KEFIR_BIGINT_DIGIT_BIT;
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_negate(__KEFIR_BIGINT_DIGIT_T *digits, __KEFIR_BIGINT_WIDTH_T width) {
    (void) __kefir_bigint_invert(digits, width);
    (void) __kefir_bigint_util_add_digit_zero_extended(digits, 1, width);

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_add(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                  const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                  __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    __KEFIR_BIGINT_UINT_T carry = 0;
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_UINT_T lhs_digit = lhs_digits[i];
        const __KEFIR_BIGINT_UINT_T rhs_digit = rhs_digits[i];

        const __KEFIR_BIGINT_UINT_T digit_sum = carry + lhs_digit + rhs_digit;
        lhs_digits[i] = (__KEFIR_BIGINT_DIGIT_T) digit_sum;
        carry = digit_sum >> __KEFIR_BIGINT_DIGIT_BIT;
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_subtract(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                       const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                       __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    __KEFIR_BIGINT_UINT_T carry = 0;
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_UINT_T lhs_digit = lhs_digits[i];
        const __KEFIR_BIGINT_UINT_T rhs_digit = rhs_digits[i];

        const __KEFIR_BIGINT_UINT_T digit_diff = ((__KEFIR_BIGINT_UINT_T) lhs_digit) - rhs_digit - carry;
        lhs_digits[i] = (__KEFIR_BIGINT_DIGIT_T) digit_diff;
        carry = (digit_diff >> __KEFIR_BIGINT_DIGIT_BIT) & 1;
    }

    return __KEFIR_BIGINT_OK;
}

#endif

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

#ifndef __KEFIR_BIGINT_MULTIPLY_H__
#define __KEFIR_BIGINT_MULTIPLY_H__

#include "kefir_bigint/base.h"

static __kefir_bigint_result_t __kefir_bigint_unsigned_multiply(__KEFIR_BIGINT_DIGIT_T *result_digits,
                                                                __KEFIR_BIGINT_DIGIT_T *result_row_digits,
                                                                const __KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                                const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                                __KEFIR_BIGINT_WIDTH_T result_width,
                                                                __KEFIR_BIGINT_WIDTH_T operand_width) {
    (void) __kefir_bigint_zero(result_digits, result_width);
    if (result_width == 0 || operand_width == 0) {
        return __KEFIR_BIGINT_OK;
    }

    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(operand_width);
    const __KEFIR_BIGINT_WIDTH_T result_total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(result_width);
    const __KEFIR_BIGINT_WIDTH_T operand_mask_offset = operand_width - (total_digits - 1) * __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_DIGIT_T operand_mask = (1ull << operand_mask_offset) - 1;
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        (void) __kefir_bigint_zero(result_row_digits, result_width);
        __KEFIR_BIGINT_DIGIT_T lhs_digit = lhs_digits[i];
        if (i + 1 == total_digits) {
            lhs_digit &= operand_mask;
        }

        for (__KEFIR_BIGINT_WIDTH_T j = 0; j < total_digits; j++) {
            const __KEFIR_BIGINT_WIDTH_T rhs_index = total_digits - j - 1;
            __KEFIR_BIGINT_DIGIT_T rhs_digit = rhs_digits[rhs_index];
            if (rhs_index + 1 == total_digits) {
                rhs_digit &= operand_mask;
            }
            __KEFIR_BIGINT_UINT_T digit_mul = ((__KEFIR_BIGINT_UINT_T) lhs_digit) * (__KEFIR_BIGINT_UINT_T) rhs_digit;
            (void) __kefir_bigint_left_shift_whole_digits(result_row_digits, 1, result_width);
            for (__KEFIR_BIGINT_WIDTH_T k = 0; k < result_total_digits && digit_mul != 0;
                 digit_mul >>= __KEFIR_BIGINT_DIGIT_BIT, k++) {
                (void) __kefir_bigint_util_add_digit_zero_extended(&result_row_digits[k],
                                                                   (__KEFIR_BIGINT_DIGIT_T) digit_mul,
                                                                   result_width - k * __KEFIR_BIGINT_DIGIT_BIT);
            }
        }
        if (i > 0) {
            (void) __kefir_bigint_left_shift_whole_digits(result_row_digits, i, result_width);
        }

        (void) __kefir_bigint_add(result_digits, result_row_digits, result_width);
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_signed_multiply(__KEFIR_BIGINT_DIGIT_T *result_digits,
                                                              __KEFIR_BIGINT_DIGIT_T *accumulator_digits,
                                                              __KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                              const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                              __KEFIR_BIGINT_WIDTH_T result_width,
                                                              __KEFIR_BIGINT_WIDTH_T operand_width) {
    (void) __kefir_bigint_zero(result_digits, result_width);
    if (result_width == 0 || operand_width == 0) {
        return __KEFIR_BIGINT_OK;
    }

    const __KEFIR_BIGINT_WIDTH_T multiplier_msb_index = (operand_width - 1) / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T multiplier_msb_offset =
        operand_width - 1 - multiplier_msb_index * __KEFIR_BIGINT_DIGIT_BIT;

    (void) __kefir_bigint_zero(accumulator_digits, operand_width);
    __KEFIR_BIGINT_UINT_T multiplier_lsb_prev_bit = 0;
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < operand_width; i++) {
        const __KEFIR_BIGINT_UINT_T multiplier_lsb_bit = lhs_digits[0] & 1;
        if (multiplier_lsb_bit && !multiplier_lsb_prev_bit) {
            (void) __kefir_bigint_subtract(accumulator_digits, rhs_digits, operand_width);
        } else if (!multiplier_lsb_bit && multiplier_lsb_prev_bit) {
            (void) __kefir_bigint_add(accumulator_digits, rhs_digits, operand_width);
        }

        multiplier_lsb_prev_bit = multiplier_lsb_bit;
        const __KEFIR_BIGINT_UINT_T accumulator_lsb = accumulator_digits[0] & 1;
        (void) __kefir_bigint_arithmetic_right_shift(accumulator_digits, 1, operand_width);
        (void) __kefir_bigint_right_shift(lhs_digits, 1, operand_width);
        lhs_digits[multiplier_msb_index] |= accumulator_lsb << multiplier_msb_offset;
    }

    (void) __kefir_bigint_or(result_digits, accumulator_digits, operand_width);
    (void) __kefir_bigint_left_shift(result_digits, operand_width, result_width);
    (void) __kefir_bigint_or(result_digits, lhs_digits, operand_width);
    (void) __kefir_bigint_cast_signed(result_digits, operand_width * 2, result_width);

    return __KEFIR_BIGINT_OK;
}

#endif

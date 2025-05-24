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
                                                              __KEFIR_BIGINT_WIDTH_T width) {
    (void) __kefir_bigint_zero(accumulator_digits, width);
    if (width == 0) {
        return __KEFIR_BIGINT_OK;
    }

    const __KEFIR_BIGINT_WIDTH_T msb_digit_index = (width - 1) / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T msb_digit_offset = width - 1 - msb_digit_index * __KEFIR_BIGINT_DIGIT_BIT;

    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < width; i++) {
        const __KEFIR_BIGINT_UINT_T lhs_msb = (lhs_digits[msb_digit_index] >> msb_digit_offset) & 1;
        (void) __kefir_bigint_left_shift(accumulator_digits, 1, width);
        (void) __kefir_bigint_left_shift(lhs_digits, 1, width);
        accumulator_digits[0] |= lhs_msb;

        (void) __kefir_bigint_subtract(accumulator_digits, rhs_digits, width);
        const __KEFIR_BIGINT_UINT_T acc_msb = (accumulator_digits[msb_digit_index] >> msb_digit_offset) & 1;
        if (acc_msb) {
            lhs_digits[0] &= ~(__KEFIR_BIGINT_DIGIT_T) 1;
            (void) __kefir_bigint_add(accumulator_digits, rhs_digits, width);
        } else {
            lhs_digits[0] |= 1;
        }
    }

    return __KEFIR_BIGINT_OK;
}

#endif

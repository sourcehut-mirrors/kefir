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

#ifndef __KEFIR_BIGINT_COMPARE_H__
#define __KEFIR_BIGINT_COMPARE_H__

#include "kefir_bigint/base.h"

static __KEFIR_BIGINT_UINT_T __kefir_bigint_unsigned_compare(const __KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                             const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                             __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    for (__KEFIR_BIGINT_WIDTH_T i = total_digits; i > 0; i--) {
        __KEFIR_BIGINT_UINT_T lhs_digit = lhs_digits[i - 1];
        __KEFIR_BIGINT_UINT_T rhs_digit = rhs_digits[i - 1];
        if (i == total_digits) {
            const __KEFIR_BIGINT_UINT_T mask_offset =
                ((width - 1) - (width - 1) / __KEFIR_BIGINT_DIGIT_BIT * __KEFIR_BIGINT_DIGIT_BIT) + 1;
            const __KEFIR_BIGINT_UINT_T mask = (1ull << mask_offset) - 1;
            lhs_digit &= mask;
            rhs_digit &= mask;
        }

        if (lhs_digit > rhs_digit) {
            return 1;
        } else if (lhs_digit < rhs_digit) {
            return -1;
        }
    }
    return 0;
}

static __KEFIR_BIGINT_UINT_T __kefir_bigint_signed_compare(const __KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                           const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                           __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T msb_index = (width - 1) / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T msb_offset = (width - 1) - msb_index * __KEFIR_BIGINT_DIGIT_BIT;

    const __KEFIR_BIGINT_UINT_T lhs_sign = (lhs_digits[msb_index] >> msb_offset) & 1;
    const __KEFIR_BIGINT_UINT_T rhs_sign = (rhs_digits[msb_index] >> msb_offset) & 1;
    if (lhs_sign && !rhs_sign) {
        return -1;
    } else if (!lhs_sign && rhs_sign) {
        return 1;
    }

    return __kefir_bigint_unsigned_compare(lhs_digits, rhs_digits, width);
}

#endif

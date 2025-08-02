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

#ifndef __KEFIR_BIGINT_BITS_H__
#define __KEFIR_BIGINT_BITS_H__

#include "kefir_bigint/base.h"

static __KEFIR_BIGINT_WIDTH_T __kefir_bigint_least_significant_nonzero(const __KEFIR_BIGINT_DIGIT_T *digits,
                                                                       __KEFIR_BIGINT_WIDTH_T width) {
    if (width == 0) {
        return 0;
    }

    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_DIGIT_T digit = digits[i];
        if (digit == 0) {
            continue;
        }

        const __KEFIR_BIGINT_WIDTH_T digit_width =
            i + 1 < total_digits ? __KEFIR_BIGINT_DIGIT_BIT : width % __KEFIR_BIGINT_DIGIT_BIT;
        for (__KEFIR_BIGINT_WIDTH_T j = 0; j < digit_width; j++) {
            if (((digit >> j) & 1) == 1) {
                return i * __KEFIR_BIGINT_DIGIT_BIT + j + 1;
            }
        }
    }

    return 0;
}

static __KEFIR_BIGINT_WIDTH_T __kefir_bigint_leading_zeros(const __KEFIR_BIGINT_DIGIT_T *digits,
                                                           __KEFIR_BIGINT_WIDTH_T width,
                                                           __KEFIR_BIGINT_WIDTH_T default_value) {
    if (width == 0) {
        return default_value;
    }

    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    __KEFIR_BIGINT_WIDTH_T count = 0;
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_DIGIT_T digit = digits[total_digits - i - 1];
        const __KEFIR_BIGINT_WIDTH_T digit_width = i > 0 ? __KEFIR_BIGINT_DIGIT_BIT : width % __KEFIR_BIGINT_DIGIT_BIT;
        for (__KEFIR_BIGINT_WIDTH_T j = 0; j < digit_width; j++) {
            const __KEFIR_BIGINT_UINT_T bit = (digit >> (digit_width - j - 1)) & 1;
            if (bit == 1) {
                return count;
            } else {
                count++;
            }
        }
    }

    return default_value;
}

static __KEFIR_BIGINT_WIDTH_T __kefir_bigint_trailing_zeros(const __KEFIR_BIGINT_DIGIT_T *digits,
                                                            __KEFIR_BIGINT_WIDTH_T width,
                                                            __KEFIR_BIGINT_WIDTH_T default_value) {
    if (width == 0) {
        return default_value;
    }

    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    __KEFIR_BIGINT_WIDTH_T count = 0;
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_DIGIT_T digit = digits[i];
        if (digit == 0) {
            count += __KEFIR_BIGINT_DIGIT_BIT;
            continue;
        }

        const __KEFIR_BIGINT_WIDTH_T digit_width =
            i + 1 < total_digits ? __KEFIR_BIGINT_DIGIT_BIT : width % __KEFIR_BIGINT_DIGIT_BIT;
        for (__KEFIR_BIGINT_WIDTH_T j = 0; j < digit_width; j++) {
            const __KEFIR_BIGINT_UINT_T bit = (digit >> j) & 1;
            if (bit == 1) {
                return count;
            } else {
                count++;
            }
        }
    }

    return default_value;
}

#endif

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

#ifndef __KEFIR_BIGINT_SHIFT_H__
#define __KEFIR_BIGINT_SHIFT_H__

#include "kefir_bigint/base.h"

static __kefir_bigint_result_t __kefir_bigint_left_shift_whole_digits(__KEFIR_BIGINT_DIGIT_T *digits,
                                                                      __KEFIR_BIGINT_UNSIGNED_VALUE_T shift,
                                                                      __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    for (__KEFIR_BIGINT_WIDTH_T i = total_digits - 1; i >= shift; i--) {
        digits[i] = digits[i - shift];
    }

    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < shift; i++) {
        digits[i] = (__KEFIR_BIGINT_DIGIT_T) 0;
    }
    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_left_shift(__KEFIR_BIGINT_DIGIT_T *digits, __KEFIR_BIGINT_WIDTH_T shift,
                                                         __KEFIR_BIGINT_WIDTH_T width) {
    if (shift > width) {
        shift = width;
    }

    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    const __KEFIR_BIGINT_UNSIGNED_VALUE_T shift_whole_digits = shift / __KEFIR_BIGINT_DIGIT_BIT;
    if (shift_whole_digits > 0) {
        (void) __kefir_bigint_left_shift_whole_digits(digits, shift_whole_digits, width);
        shift -= shift_whole_digits * __KEFIR_BIGINT_DIGIT_BIT;
    }

    if (shift > 0) {
        for (__KEFIR_BIGINT_WIDTH_T i = total_digits - 1; i > 0; i--) {
            digits[i] = (digits[i] << shift) | (digits[i - 1] >> (__KEFIR_BIGINT_DIGIT_BIT - shift));
        }
        digits[0] <<= shift;
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_right_shift_whole_digits(__KEFIR_BIGINT_DIGIT_T *digits,
                                                                       __KEFIR_BIGINT_UNSIGNED_VALUE_T shift,
                                                                       __KEFIR_BIGINT_UINT_T sign,
                                                                       __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    const __KEFIR_BIGINT_DIGIT_T sign_extension = sign ? ~(__KEFIR_BIGINT_DIGIT_T) 0ull : (__KEFIR_BIGINT_DIGIT_T) 0ull;
    if (shift >= total_digits) {
        for (__KEFIR_BIGINT_UINT_T i = 0; i < total_digits; i++) {
            digits[i] = sign_extension;
        }
    } else {
        __KEFIR_BIGINT_UINT_T i = 0;
        for (; i < total_digits - shift; i++) {
            digits[i] = digits[i + shift];
        }
        for (; i < total_digits; i++) {
            digits[i] = sign_extension;
        }
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_right_shift(__KEFIR_BIGINT_DIGIT_T *digits, __KEFIR_BIGINT_WIDTH_T shift,
                                                          __KEFIR_BIGINT_WIDTH_T width) {
    if (shift > width) {
        shift = width;
    }

    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    const __KEFIR_BIGINT_UNSIGNED_VALUE_T shift_whole_digits = shift / __KEFIR_BIGINT_DIGIT_BIT;
    if (shift_whole_digits > 0) {
        (void) __kefir_bigint_right_shift_whole_digits(digits, shift_whole_digits, 0, width);
        shift -= shift_whole_digits * __KEFIR_BIGINT_DIGIT_BIT;
    }

    if (shift > 0) {
        for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits - 1; i++) {
            digits[i] = (digits[i] >> shift) | (digits[i + 1] << (__KEFIR_BIGINT_DIGIT_BIT - shift));
        }

        const __KEFIR_BIGINT_WIDTH_T mask_offset = width - (total_digits - 1) * __KEFIR_BIGINT_DIGIT_BIT;
        const __KEFIR_BIGINT_UINT_T mask = (1ull << mask_offset) - 1;

        digits[total_digits - 1] &= mask;
        digits[total_digits - 1] >>= shift;
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_arithmetic_right_shift(__KEFIR_BIGINT_DIGIT_T *digits,
                                                                     __KEFIR_BIGINT_WIDTH_T shift,
                                                                     __KEFIR_BIGINT_WIDTH_T width) {
    if (width == 0) {
        return __KEFIR_BIGINT_OK;
    }
    if (shift > width) {
        shift = width;
    }

    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    const __KEFIR_BIGINT_WIDTH_T msb_digit_index = (width - 1) / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T msb_digit_offset = (width - 1) - msb_digit_index * __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T sign = (digits[msb_digit_index] >> msb_digit_offset) & 1;

    const __KEFIR_BIGINT_UNSIGNED_VALUE_T shift_whole_digits = shift / __KEFIR_BIGINT_DIGIT_BIT;
    if (shift_whole_digits > 0) {
        (void) __kefir_bigint_right_shift_whole_digits(digits, shift_whole_digits, sign, width);
        shift -= shift_whole_digits * __KEFIR_BIGINT_DIGIT_BIT;
    }

    if (shift > 0) {
        for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits - 1; i++) {
            digits[i] = (digits[i] >> shift) | (digits[i + 1] << (__KEFIR_BIGINT_DIGIT_BIT - shift));
        }

        digits[total_digits - 1] >>= shift;

        const __KEFIR_BIGINT_WIDTH_T mask_offset = msb_digit_offset >= shift ? msb_digit_offset - shift + 1 : 0;
        if (sign) {
            const __KEFIR_BIGINT_UINT_T mask = ~((1ull << mask_offset) - 1);
            digits[total_digits - 1] |= mask;
        } else {
            const __KEFIR_BIGINT_UINT_T mask = (1ull << mask_offset) - 1;
            digits[total_digits - 1] &= mask;
        }
    }

    return __KEFIR_BIGINT_OK;
}

#endif

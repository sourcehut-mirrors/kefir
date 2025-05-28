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

#ifndef __KEFIR_BIGINT_GET_H__
#define __KEFIR_BIGINT_GET_H__

#include "kefir_bigint/base.h"

static __KEFIR_BIGINT_WIDTH_T __kefir_bigint_native_signed_width(__KEFIR_BIGINT_SIGNED_VALUE_T value) {
    __KEFIR_BIGINT_UNSIGNED_VALUE_T uvalue = (__KEFIR_BIGINT_UNSIGNED_VALUE_T) value;
    const __KEFIR_BIGINT_UNSIGNED_VALUE_T mask = (1ull << (__KEFIR_BIGINT_VALUE_BIT - 1));
    const __KEFIR_BIGINT_UNSIGNED_VALUE_T msb = uvalue & mask;
    __KEFIR_BIGINT_WIDTH_T bits = __KEFIR_BIGINT_VALUE_BIT + 1;
    for (; bits > 1 && (uvalue & mask) == msb; uvalue <<= 1, bits--) {}
    return bits;
}

static __KEFIR_BIGINT_WIDTH_T __kefir_bigint_native_unsigned_width(__KEFIR_BIGINT_UNSIGNED_VALUE_T value) {
    __KEFIR_BIGINT_WIDTH_T bits = 0;
    for (; value != 0; value >>= 1, bits++) {}
    if (bits == 0) {
        bits = 1;
    }
    return bits;
}

static __kefir_bigint_result_t __kefir_bigint_get_signed_value(const __KEFIR_BIGINT_DIGIT_T *digits,
                                                               __KEFIR_BIGINT_WIDTH_T width,
                                                               __KEFIR_BIGINT_SIGNED_VALUE_T *value_ptr) {
    if (width > __KEFIR_BIGINT_VALUE_BIT) {
        width = __KEFIR_BIGINT_VALUE_BIT;
    }
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    __KEFIR_BIGINT_UNSIGNED_VALUE_T value = 0;
    for (__KEFIR_BIGINT_WIDTH_T i = total_digits; i > 0; i--) {
        value <<= __KEFIR_BIGINT_DIGIT_BIT;
        value |= digits[i - 1];
    }

    if (width < __KEFIR_BIGINT_VALUE_BIT) {
        const __KEFIR_BIGINT_UINT_T sign = width > 0 ? (value >> (width - 1)) & 1 : 0;

        if (sign) {
            const unsigned long long mask = ~((1ull << width) - 1);
            value |= mask;
        } else {
            const unsigned long long mask = (1ull << width) - 1;
            value &= mask;
        }
    }

    *value_ptr = value;
    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_get_unsigned_value(const __KEFIR_BIGINT_DIGIT_T *digits,
                                                                 __KEFIR_BIGINT_WIDTH_T width,
                                                                 __KEFIR_BIGINT_UNSIGNED_VALUE_T *value_ptr) {
    if (width > __KEFIR_BIGINT_VALUE_BIT) {
        width = __KEFIR_BIGINT_VALUE_BIT;
    }
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    __KEFIR_BIGINT_UNSIGNED_VALUE_T value = 0;
    for (__KEFIR_BIGINT_WIDTH_T i = total_digits; i > 0; i--) {
        value <<= __KEFIR_BIGINT_DIGIT_BIT;
        value |= digits[i - 1];
    }

    if (width < __KEFIR_BIGINT_VALUE_BIT) {
        const unsigned long long mask = (1ull << width) - 1;
        value &= mask;
    }

    *value_ptr = value;
    return __KEFIR_BIGINT_OK;
}

static __KEFIR_BIGINT_UINT_T __kefir_bigint_get_sign(const __KEFIR_BIGINT_DIGIT_T *digits,
                                                     __KEFIR_BIGINT_WIDTH_T width) {
    if (width == 0) {
        return 0;
    }

    const __KEFIR_BIGINT_WIDTH_T msb_index = (width - 1) / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T msb_offset = width - 1 - msb_index * __KEFIR_BIGINT_DIGIT_BIT;
    return (digits[msb_index] >> msb_offset) & 1;
}

static __KEFIR_BIGINT_UINT_T __kefir_bigint_is_zero(const __KEFIR_BIGINT_DIGIT_T *digits,
                                                    __KEFIR_BIGINT_WIDTH_T width) {
    if (width == 0) {
        return __KEFIR_BIGINT_TRUE;
    }

    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        __KEFIR_BIGINT_DIGIT_T digit = digits[i];
        if (i + 1 == total_digits) {
            const __KEFIR_BIGINT_WIDTH_T msb_index = (width - 1) / __KEFIR_BIGINT_DIGIT_BIT;
            const __KEFIR_BIGINT_WIDTH_T msb_offset = width - 1 - msb_index * __KEFIR_BIGINT_DIGIT_BIT;
            const __KEFIR_BIGINT_UINT_T mask = (1ull << (msb_offset + 1)) - 1;
            digit &= mask;
        }
        if (digit) {
            return __KEFIR_BIGINT_FALSE;
        }
    }
    return __KEFIR_BIGINT_TRUE;
}

static __KEFIR_BIGINT_UNSIGNED_VALUE_T __kefir_bigint_get_bits(__KEFIR_BIGINT_DIGIT_T *digits,
                                                       __KEFIR_BIGINT_WIDTH_T offset, __KEFIR_BIGINT_WIDTH_T length,
                                                       __KEFIR_BIGINT_WIDTH_T width) {
    __KEFIR_BIGINT_WIDTH_T iter = offset;
    __KEFIR_BIGINT_WIDTH_T end = offset + length;
    if (end > width) {
        end = width;
    }

    __KEFIR_BIGINT_UNSIGNED_VALUE_T value = 0;
    for (; iter < end;) {
        __KEFIR_BIGINT_WIDTH_T chunk_end =
            iter % __KEFIR_BIGINT_DIGIT_BIT != 0
                ? (iter + __KEFIR_BIGINT_DIGIT_BIT - 1) / __KEFIR_BIGINT_DIGIT_BIT * __KEFIR_BIGINT_DIGIT_BIT
                : iter + __KEFIR_BIGINT_DIGIT_BIT;
        if (chunk_end > end) {
            chunk_end = end;
        }
        const __KEFIR_BIGINT_WIDTH_T chunk_width = chunk_end - iter;
        const __KEFIR_BIGINT_UINT_T chunk_mask = (1ull << chunk_width) - 1;
        const __KEFIR_BIGINT_WIDTH_T target_index = iter / __KEFIR_BIGINT_DIGIT_BIT;
        const __KEFIR_BIGINT_WIDTH_T target_offset = iter - target_index * __KEFIR_BIGINT_DIGIT_BIT;
        const __KEFIR_BIGINT_UNSIGNED_VALUE_T chunk = (digits[target_index] >> target_offset) & chunk_mask;

        value |= chunk << (iter - offset);
        iter = chunk_end;
    }

    return value;
}

#endif

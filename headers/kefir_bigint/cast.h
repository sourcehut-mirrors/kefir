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

#ifndef __KEFIR_BIGINT_CAST_H__
#define __KEFIR_BIGINT_CAST_H__

#include "kefir_bigint/base.h"

static __kefir_bigint_result_t __kefir_bigint_cast_signed(__KEFIR_BIGINT_DIGIT_T *digits,
                                                          __KEFIR_BIGINT_WIDTH_T current_width,
                                                          __KEFIR_BIGINT_WIDTH_T desired_width) {
    const __KEFIR_BIGINT_WIDTH_T desired_msb_location = desired_width - 1;
    const __KEFIR_BIGINT_WIDTH_T desired_msb_digit_index = desired_msb_location / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T desired_msb_bit_offset =
        desired_msb_location - desired_msb_digit_index * __KEFIR_BIGINT_DIGIT_BIT;

    if (desired_width <= current_width) {
        __KEFIR_BIGINT_UINT_T desired_msb = (digits[desired_msb_digit_index] >> desired_msb_bit_offset) & 1;
        if (desired_msb) {
            __KEFIR_BIGINT_UINT_T mask = ~((1ull << desired_msb_bit_offset) - 1);
            digits[desired_msb_digit_index] |= mask;
        } else {
            __KEFIR_BIGINT_UINT_T mask = (1ull << desired_msb_bit_offset) - 1;
            digits[desired_msb_digit_index] &= mask;
        }
        return __KEFIR_BIGINT_OK;
    }

    if (current_width == 0) {
        for (__KEFIR_BIGINT_WIDTH_T i = 0; i <= desired_msb_digit_index; i++) {
            digits[i] = (__KEFIR_BIGINT_DIGIT_T) 0;
        }
        return __KEFIR_BIGINT_OK;
    }

    const __KEFIR_BIGINT_WIDTH_T current_msb_location = current_width - 1;
    const __KEFIR_BIGINT_WIDTH_T current_msb_digit_index = current_msb_location / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T current_msb_bit_offset =
        current_msb_location - current_msb_digit_index * __KEFIR_BIGINT_DIGIT_BIT;

    __KEFIR_BIGINT_UINT_T current_msb = (digits[current_msb_digit_index] >> current_msb_bit_offset) & 1;
    if (current_msb_bit_offset + 1 < __KEFIR_BIGINT_DIGIT_BIT) {
        if (current_msb) {
            __KEFIR_BIGINT_UINT_T mask = ~((1ull << current_msb_bit_offset) - 1);
            digits[current_msb_digit_index] |= mask;
        } else {
            __KEFIR_BIGINT_UINT_T mask = (1ull << current_msb_bit_offset) - 1;
            digits[current_msb_digit_index] &= mask;
        }
    }

    for (__KEFIR_BIGINT_WIDTH_T i = current_msb_digit_index + 1; i <= desired_msb_digit_index; i++) {
        if (current_msb) {
            digits[i] = ~(__KEFIR_BIGINT_DIGIT_T) 0ull;
        } else {
            digits[i] = (__KEFIR_BIGINT_DIGIT_T) 0ull;
        }
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_cast_unsigned(__KEFIR_BIGINT_DIGIT_T *digits,
                                                            __KEFIR_BIGINT_WIDTH_T current_width,
                                                            __KEFIR_BIGINT_WIDTH_T desired_width) {
    const __KEFIR_BIGINT_WIDTH_T desired_msb_location = desired_width - 1;
    const __KEFIR_BIGINT_WIDTH_T desired_msb_digit_index = desired_msb_location / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T desired_msb_bit_offset =
        desired_width - desired_msb_digit_index * __KEFIR_BIGINT_DIGIT_BIT;

    if (desired_width <= current_width) {
        __KEFIR_BIGINT_UINT_T mask = (1ull << desired_msb_bit_offset) - 1;
        digits[desired_msb_digit_index] &= mask;
        return __KEFIR_BIGINT_OK;
    }

    if (current_width == 0) {
        for (__KEFIR_BIGINT_WIDTH_T i = 0; i <= desired_msb_digit_index; i++) {
            digits[i] = (__KEFIR_BIGINT_DIGIT_T) 0ull;
        }
        return __KEFIR_BIGINT_OK;
    }

    const __KEFIR_BIGINT_WIDTH_T current_msb_location = current_width - 1;
    const __KEFIR_BIGINT_WIDTH_T current_msb_digit_index = current_msb_location / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T current_msb_bit_offset =
        current_width - current_msb_digit_index * __KEFIR_BIGINT_DIGIT_BIT;

    if (current_msb_bit_offset + 1 < __KEFIR_BIGINT_DIGIT_BIT) {
        __KEFIR_BIGINT_UINT_T mask = (1ull << current_msb_bit_offset) - 1;
        digits[current_msb_digit_index] &= mask;
    }

    for (__KEFIR_BIGINT_WIDTH_T i = current_msb_digit_index + 1; i <= desired_msb_digit_index; i++) {
        digits[i] = (__KEFIR_BIGINT_DIGIT_T) 0ull;
    }

    return __KEFIR_BIGINT_OK;
}

#endif

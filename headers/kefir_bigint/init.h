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

#ifndef __KEFIR_BIGINT_INIT_H__
#define __KEFIR_BIGINT_INIT_H__

#include "kefir_bigint/base.h"

static __kefir_bigint_result_t __kefir_bigint_set_signed_integer(__KEFIR_BIGINT_DIGIT_T *digits,
                                                                 __KEFIR_BIGINT_WIDTH_T width,
                                                                 __KEFIR_BIGINT_UNSIGNED_VALUE_T value) {
    if (width == 0) {
        return __KEFIR_BIGINT_OK;
    }

    __KEFIR_BIGINT_WIDTH_T fit_width = width;
    if (fit_width > __KEFIR_BIGINT_VALUE_BIT) {
        fit_width = __KEFIR_BIGINT_VALUE_BIT;
    }

    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(fit_width);
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++, value >>= __KEFIR_BIGINT_DIGIT_BIT) {
        digits[i] = (__KEFIR_BIGINT_DIGIT_T) value;
    }

    if (fit_width < width) {
        (void) __kefir_bigint_cast_signed(digits, fit_width, width);
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_set_unsigned_integer(__KEFIR_BIGINT_DIGIT_T *digits,
                                                                   __KEFIR_BIGINT_WIDTH_T width,
                                                                   __KEFIR_BIGINT_UNSIGNED_VALUE_T value) {
    if (width == 0) {
        return __KEFIR_BIGINT_OK;
    }

    __KEFIR_BIGINT_WIDTH_T fit_width = width;
    if (fit_width > __KEFIR_BIGINT_VALUE_BIT) {
        fit_width = __KEFIR_BIGINT_VALUE_BIT;
    }

    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(fit_width);
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++, value >>= __KEFIR_BIGINT_DIGIT_BIT) {
        digits[i] = (__KEFIR_BIGINT_DIGIT_T) value;
    }

    if (fit_width < width) {
        (void) __kefir_bigint_cast_unsigned(digits, fit_width, width);
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_zero(__KEFIR_BIGINT_DIGIT_T *digits, __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        digits[i] = (__KEFIR_BIGINT_DIGIT_T) 0;
    }
    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_copy(__KEFIR_BIGINT_DIGIT_T *dst_digits,
                                                   const __KEFIR_BIGINT_DIGIT_T *src_digits,
                                                   __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        dst_digits[i] = src_digits[i];
    }
    return __KEFIR_BIGINT_OK;
}

#endif

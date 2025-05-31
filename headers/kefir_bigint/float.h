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

#ifndef __KEFIR_BIGINT_FLOAT_H__
#define __KEFIR_BIGINT_FLOAT_H__

#include "kefir_bigint/base.h"

#define __KEFIR_BIGINT_SIGNED_TO_IEEE754_IMPL(_digits, _tmp_digits, _width, _mant_dig, _sign, _exponent, _mantissa)   \
    do {                                                                                                              \
        *(_sign) = -(__KEFIR_BIGINT_SIGNED_VALUE_T) __kefir_bigint_get_sign((_digits), (_width));                     \
        if (sign) {                                                                                                   \
            (void) __kefir_bigint_negate((_digits), (_width));                                                        \
        }                                                                                                             \
                                                                                                                      \
        const __KEFIR_BIGINT_WIDTH_T significant_digits = __kefir_bigint_get_min_unsigned_width((_digits), (_width)); \
        *(_exponent) = significant_digits - 1;                                                                        \
        if (significant_digits > (_mant_dig)) {                                                                       \
            switch (significant_digits) {                                                                             \
                case (_mant_dig) + 1:                                                                                 \
                    (void) __kefir_bigint_left_shift((_digits), 1, (_width));                                         \
                    break;                                                                                            \
                                                                                                                      \
                case (_mant_dig) + 2:                                                                                 \
                    /* Intentionally left blank */                                                                    \
                    break;                                                                                            \
                                                                                                                      \
                default:                                                                                              \
                    (void) __kefir_bigint_set_signed_integer((_tmp_digits), (_width), -1);                            \
                    (void) __kefir_bigint_right_shift((_tmp_digits), (_width) + (_mant_dig) + 2 - significant_digits, \
                                                      (_width));                                                      \
                    (void) __kefir_bigint_and((_tmp_digits), (_digits), (_width));                                    \
                    (void) __kefir_bigint_set_unsigned_integer((_tmp_digits), (_width),                               \
                                                               !__kefir_bigint_is_zero((_tmp_digits), (_width)));     \
                    (void) __kefir_bigint_right_shift((_digits), significant_digits - ((_mant_dig) + 2), (_width));   \
                    (void) __kefir_bigint_or((_digits), (_tmp_digits), (_width));                                     \
                    break;                                                                                            \
            }                                                                                                         \
                                                                                                                      \
            __KEFIR_BIGINT_UNSIGNED_VALUE_T val;                                                                      \
            (void) __kefir_bigint_get_unsigned_value((_digits), (_width), &val);                                      \
            (void) __kefir_bigint_set_unsigned_integer((_tmp_digits), (_width), (val & 4) != 0);                      \
            (void) __kefir_bigint_or((_digits), (_tmp_digits), (_width));                                             \
            (void) __kefir_bigint_util_add_digit_zero_extended((_digits), 1, (_width));                               \
            (void) __kefir_bigint_right_shift((_digits), 2, (_width));                                                \
                                                                                                                      \
            if (__kefir_bigint_get_bits((_digits), (_mant_dig), 1, (_width))) {                                       \
                (void) __kefir_bigint_right_shift((_digits), 1, (_width));                                            \
                (*(_exponent))++;                                                                                     \
            }                                                                                                         \
                                                                                                                      \
        } else if (significant_digits != (_mant_dig)) {                                                               \
            (void) __kefir_bigint_left_shift((_digits), (_mant_dig) - significant_digits, (_width));                  \
        }                                                                                                             \
                                                                                                                      \
        (void) __kefir_bigint_get_unsigned_value((_digits), (_width), (_mantissa));                                   \
    } while (0)

static __KEFIR_BIGINT_FLOAT_T __kefir_bigint_signed_to_float(__KEFIR_BIGINT_DIGIT_T *signed_digits,
                                                             __KEFIR_BIGINT_DIGIT_T *tmp,
                                                             __KEFIR_BIGINT_WIDTH_T width) {
    // Conversion algorithm is taken from
    // https://github.com/llvm-mirror/compiler-rt/blob/69445f095c22aac2388f939bedebf224a6efcdaf/lib/builtins/floattisf.c#L24C23-L24C34

    if (__kefir_bigint_is_zero(signed_digits, width)) {
        return 0.0f;
    }

    __KEFIR_BIGINT_UNSIGNED_VALUE_T sign, exponent, mantissa;
    __KEFIR_BIGINT_SIGNED_TO_IEEE754_IMPL(signed_digits, tmp, width, __KEFIR_BIGINT_FLT_MANT_DIG, &sign, &exponent,
                                          &mantissa);
    union {
        __KEFIR_BIGINT_UNSIGNED_VALUE_T u;
        __KEFIR_BIGINT_FLOAT_T f;
    } conv = {.u = (sign & 0x80000000) | ((exponent + 127) << 23) | (mantissa & 0x007fffff)};
    return conv.f;
}

static __KEFIR_BIGINT_DOUBLE_T __kefir_bigint_signed_to_double(__KEFIR_BIGINT_DIGIT_T *signed_digits,
                                                               __KEFIR_BIGINT_DIGIT_T *tmp,
                                                               __KEFIR_BIGINT_WIDTH_T width) {
    if (__kefir_bigint_is_zero(signed_digits, width)) {
        return 0.0;
    }

    __KEFIR_BIGINT_UNSIGNED_VALUE_T sign, exponent, mantissa;
    __KEFIR_BIGINT_SIGNED_TO_IEEE754_IMPL(signed_digits, tmp, width, __KEFIR_BIGINT_DBL_MANT_DIG, &sign, &exponent,
                                          &mantissa);
    union {
        __KEFIR_BIGINT_UNSIGNED_VALUE_T u;
        __KEFIR_BIGINT_DOUBLE_T f;
    } conv = {.u = ((sign & 0x80000000) << 32) | ((exponent + 1023) << 52) | (mantissa & 0x000fffffffffffffull)};
    return conv.f;
}

#undef __KEFIR_BIGINT_SIGNED_TO_IEEE754_IMPL

#endif

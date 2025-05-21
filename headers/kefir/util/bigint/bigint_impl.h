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

#ifndef KEFIR_UTIL_BIGINT_BIGINT_IMPL_H_
#define KEFIR_UTIL_BIGINT_BIGINT_IMPL_H_

#ifndef __KEFIR_BIGINT_USE_BIGINT_IMPL__
#error "bigint_impl.h header shall not be included directly"
#endif

#ifndef __KEFIR_BIGINT_CHAR_BIT
#error "bigint_impl.h environment is missing __KEFIR_BIGINT_CHAR_BIT definition"
#endif

#ifndef __KEFIR_BIGINT_SIGNED_VALUE_TYPE
#define __KEFIR_BIGINT_SIGNED_VALUE_TYPE long long
#endif

#ifndef __KEFIR_BIGINT_UNSIGNED_VALUE_TYPE
#define __KEFIR_BIGINT_UNSIGNED_VALUE_TYPE unsigned __KEFIR_BIGINT_SIGNED_VALUE_TYPE
#endif

#ifndef __KEFIR_BIGINT_DIGIT_TYPE
#define __KEFIR_BIGINT_DIGIT_TYPE unsigned char
#endif

#define __KEFIR_BIGINT_SIGNED_INTEGER_BITS (sizeof(__KEFIR_BIGINT_SIGNED_VALUE_TYPE) * __KEFIR_BIGINT_CHAR_BIT)
#define __KEFIR_BIGINT_DIGIT_BITS (sizeof(__KEFIR_BIGINT_DIGIT_TYPE) * __KEFIR_BIGINT_CHAR_BIT)

typedef enum { __KEFIR_BIGINT_OK } __kefir_bigint_result_t;

#define __KEFIR_BIGINT_TRUE 1
#define __KEFIR_BIGINT_FALSE 0
#define __KEFIR_BIGINT_NULL ((void *) 0)

#define __KEFIR_BIGINT_BITS_TO_DIGITS(_bits) (((_bits) + __KEFIR_BIGINT_DIGIT_BITS - 1) / __KEFIR_BIGINT_DIGIT_BITS)

static __KEFIR_BIGINT_UNSIGNED_VALUE_TYPE __kefir_bigint_count_nonzero_bits(__KEFIR_BIGINT_UNSIGNED_VALUE_TYPE value) {
    __KEFIR_BIGINT_UNSIGNED_VALUE_TYPE bits = 0;
    for (; value != 0; value >>= 1, bits++) {}
    if (bits == 0) {
        bits = 1;
    }
    return bits;
}

static __kefir_bigint_result_t __kefir_bigint_set_signed_integer(__KEFIR_BIGINT_DIGIT_TYPE *digits, __KEFIR_BIGINT_UNSIGNED_VALUE_TYPE maxwidth,
                                                          __KEFIR_BIGINT_UNSIGNED_VALUE_TYPE *width_ptr, __KEFIR_BIGINT_SIGNED_VALUE_TYPE value) {
    if (maxwidth == 0) {
        return __KEFIR_BIGINT_OK;
    }
    unsigned int total_bitwidth = __kefir_bigint_count_nonzero_bits(value) + (value > 0 ? 1 : 0);
    if (total_bitwidth > maxwidth) {
        total_bitwidth = maxwidth;
    }
    const unsigned int total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(total_bitwidth);

    for (unsigned int i = 0; i < total_digits; i++, value >>= __KEFIR_BIGINT_DIGIT_BITS) {
        digits[i] = (unsigned char) value;
    }

    if (width_ptr != __KEFIR_BIGINT_NULL) {
        *width_ptr = total_bitwidth;
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_get_signed_value(const unsigned char *digits, unsigned long width,
                                                          long long *value_ptr) {
    if (width > __KEFIR_BIGINT_SIGNED_INTEGER_BITS) {
        width = __KEFIR_BIGINT_SIGNED_INTEGER_BITS;
    }
    const unsigned long total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    unsigned long long uvalue = 0;
    for (unsigned long i = total_digits; i > 0; i--) {
        uvalue <<= __KEFIR_BIGINT_DIGIT_BITS;
        uvalue |= digits[i - 1];
    }

    if (width < __KEFIR_BIGINT_SIGNED_INTEGER_BITS) {
        const unsigned int sign = width > 0
            ? (uvalue >> (width - 1)) & 1
            : 0;

        if (sign) {
            const unsigned long long mask = ~((1ull << width) - 1);
            uvalue |= mask;
        } else {
            const unsigned long long mask = (1ull << width) - 1;
            uvalue &= mask;
        }
    }

    if (value_ptr != __KEFIR_BIGINT_NULL) {
        *value_ptr = uvalue;
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_cast_signed(__KEFIR_BIGINT_DIGIT_TYPE *digits, __KEFIR_BIGINT_UNSIGNED_VALUE_TYPE current_width,
                                                          __KEFIR_BIGINT_UNSIGNED_VALUE_TYPE desired_width) {
    if (desired_width <= current_width) {
        return __KEFIR_BIGINT_OK;
    }

    const unsigned int desired_msb_location = desired_width - 1;
    const unsigned int desired_msb_digit = desired_msb_location / __KEFIR_BIGINT_DIGIT_BITS;

    if (current_width == 0) {
        for (unsigned long i = 0; i <= desired_msb_digit; i++) {
            digits[i] = (__KEFIR_BIGINT_DIGIT_TYPE) 0;
        }
        return __KEFIR_BIGINT_OK;
    }

    const unsigned int current_msb_location = current_width - 1;
    const unsigned int current_msb_digit = current_msb_location / __KEFIR_BIGINT_DIGIT_BITS;
    const unsigned int current_msb_index = current_msb_location - current_msb_digit * __KEFIR_BIGINT_DIGIT_BITS;

    unsigned int current_msb = (digits[current_msb_digit] >> current_msb_index) & 1;
    if (current_msb_index + 1 < __KEFIR_BIGINT_DIGIT_BITS) {
        if (current_msb) {
            const unsigned long long mask = ~((1ull << current_msb_index) - 1);
            digits[current_msb_digit] |= mask;
        } else {
            const unsigned long long mask = (1ull << current_msb_index) - 1;
            digits[current_msb_digit] &= mask;
        }
    }

    for (unsigned int i = current_msb_digit + 1; i <= desired_msb_digit; i++) {
        if (current_msb) {
            digits[i] = ~(__KEFIR_BIGINT_DIGIT_TYPE) 0ull;
        } else {
            digits[i] = (__KEFIR_BIGINT_DIGIT_TYPE) 0ull;
        }
    }

    return __KEFIR_BIGINT_OK;
}

#endif

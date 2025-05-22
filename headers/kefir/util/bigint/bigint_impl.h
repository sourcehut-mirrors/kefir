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

#ifndef __KEFIR_BIGINT_SIGNED_VALUE_T
#define __KEFIR_BIGINT_SIGNED_VALUE_T long long
#endif

#ifndef __KEFIR_BIGINT_UNSIGNED_VALUE_T
#define __KEFIR_BIGINT_UNSIGNED_VALUE_T unsigned __KEFIR_BIGINT_SIGNED_VALUE_T
#endif

#ifndef __KEFIR_BIGINT_DIGIT_T
#define __KEFIR_BIGINT_DIGIT_T unsigned char
#endif

#ifndef __KEFIR_BIGINT_INT_T
#define __KEFIR_BIGINT_INT_T unsigned int
#endif

#define __KEFIR_BIGINT_VALUE_BIT (sizeof(__KEFIR_BIGINT_SIGNED_VALUE_T) * __KEFIR_BIGINT_CHAR_BIT)
#define __KEFIR_BIGINT_DIGIT_BIT (sizeof(__KEFIR_BIGINT_DIGIT_T) * __KEFIR_BIGINT_CHAR_BIT)

typedef enum { __KEFIR_BIGINT_OK } __kefir_bigint_result_t;

#define __KEFIR_BIGINT_TRUE 1
#define __KEFIR_BIGINT_FALSE 0
#define __KEFIR_BIGINT_NULL ((void *) 0)

#define __KEFIR_BIGINT_BITS_TO_DIGITS(_bits) (((_bits) + __KEFIR_BIGINT_DIGIT_BIT - 1) / __KEFIR_BIGINT_DIGIT_BIT)

static __KEFIR_BIGINT_INT_T __kefir_bigint_count_nonzero_bits(__KEFIR_BIGINT_UNSIGNED_VALUE_T value) {
    __KEFIR_BIGINT_INT_T bits = 0;
    for (; value != 0; value >>= 1, bits++) {}
    if (bits == 0) {
        bits = 1;
    }
    return bits;
}

static __kefir_bigint_result_t __kefir_bigint_set_signed_integer(__KEFIR_BIGINT_DIGIT_T *digits,
                                                                 __KEFIR_BIGINT_INT_T width,
                                                                 __KEFIR_BIGINT_INT_T *width_ptr,
                                                                 __KEFIR_BIGINT_SIGNED_VALUE_T value) {
    if (width == 0) {
        return __KEFIR_BIGINT_OK;
    }
    __KEFIR_BIGINT_INT_T fit_width = __kefir_bigint_count_nonzero_bits(value) + (value > 0 ? 1 : 0);
    if (fit_width > width) {
        fit_width = width;
    }
    const __KEFIR_BIGINT_INT_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(fit_width);

    for (__KEFIR_BIGINT_INT_T i = 0; i < total_digits; i++, value >>= __KEFIR_BIGINT_DIGIT_BIT) {
        digits[i] = (unsigned char) value;
    }

    *width_ptr = fit_width;
    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_get_signed_value(const unsigned char *digits, __KEFIR_BIGINT_INT_T width,
                                                               __KEFIR_BIGINT_SIGNED_VALUE_T *value_ptr) {
    if (width > __KEFIR_BIGINT_VALUE_BIT) {
        width = __KEFIR_BIGINT_VALUE_BIT;
    }
    const __KEFIR_BIGINT_INT_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    __KEFIR_BIGINT_UNSIGNED_VALUE_T value = 0;
    for (__KEFIR_BIGINT_INT_T i = total_digits; i > 0; i--) {
        value <<= __KEFIR_BIGINT_DIGIT_BIT;
        value |= digits[i - 1];
    }

    if (width < __KEFIR_BIGINT_VALUE_BIT) {
        const __KEFIR_BIGINT_INT_T sign = width > 0 ? (value >> (width - 1)) & 1 : 0;

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

static __kefir_bigint_result_t __kefir_bigint_cast_signed(__KEFIR_BIGINT_DIGIT_T *digits,
                                                          __KEFIR_BIGINT_INT_T current_width,
                                                          __KEFIR_BIGINT_INT_T desired_width) {
    const __KEFIR_BIGINT_INT_T desired_msb_location = desired_width - 1;
    const __KEFIR_BIGINT_INT_T desired_msb_digit_index = desired_msb_location / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_INT_T desired_msb_bit_offset =
        desired_msb_location - desired_msb_digit_index * __KEFIR_BIGINT_DIGIT_BIT;

    if (desired_width <= current_width) {
        __KEFIR_BIGINT_INT_T desired_msb = (digits[desired_msb_digit_index] >> desired_msb_bit_offset) & 1;
        if (desired_msb) {
            const unsigned long long mask = ~((1ull << desired_msb_bit_offset) - 1);
            digits[desired_msb_digit_index] |= mask;
        } else {
            const unsigned long long mask = (1ull << desired_msb_bit_offset) - 1;
            digits[desired_msb_digit_index] &= mask;
        }
        return __KEFIR_BIGINT_OK;
    }

    if (current_width == 0) {
        for (unsigned long i = 0; i <= desired_msb_digit_index; i++) {
            digits[i] = (__KEFIR_BIGINT_DIGIT_T) 0;
        }
        return __KEFIR_BIGINT_OK;
    }

    const __KEFIR_BIGINT_INT_T current_msb_location = current_width - 1;
    const __KEFIR_BIGINT_INT_T current_msb_digit_index = current_msb_location / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_INT_T current_msb_bit_offset =
        current_msb_location - current_msb_digit_index * __KEFIR_BIGINT_DIGIT_BIT;

    __KEFIR_BIGINT_INT_T current_msb = (digits[current_msb_digit_index] >> current_msb_bit_offset) & 1;
    if (current_msb_bit_offset + 1 < __KEFIR_BIGINT_DIGIT_BIT) {
        if (current_msb) {
            const unsigned long long mask = ~((1ull << current_msb_bit_offset) - 1);
            digits[current_msb_digit_index] |= mask;
        } else {
            const unsigned long long mask = (1ull << current_msb_bit_offset) - 1;
            digits[current_msb_digit_index] &= mask;
        }
    }

    for (__KEFIR_BIGINT_INT_T i = current_msb_digit_index + 1; i <= desired_msb_digit_index; i++) {
        if (current_msb) {
            digits[i] = ~(__KEFIR_BIGINT_DIGIT_T) 0ull;
        } else {
            digits[i] = (__KEFIR_BIGINT_DIGIT_T) 0ull;
        }
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_add(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                  const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                  __KEFIR_BIGINT_UNSIGNED_VALUE_T width) {
    const __KEFIR_BIGINT_INT_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    __KEFIR_BIGINT_INT_T carry = 0;
    for (__KEFIR_BIGINT_INT_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_INT_T lhs_digit = lhs_digits[i];
        const __KEFIR_BIGINT_INT_T rhs_digit = rhs_digits[i];

        const __KEFIR_BIGINT_INT_T digit_sum = carry + lhs_digit + rhs_digit;
        lhs_digits[i] = (__KEFIR_BIGINT_DIGIT_T) digit_sum;
        carry = digit_sum >> __KEFIR_BIGINT_DIGIT_BIT;
    }

    return __KEFIR_BIGINT_OK;
}

#endif

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

#ifndef __KEFIR_BIGINT_UINT_T
#define __KEFIR_BIGINT_UINT_T unsigned int
#endif

#ifndef __KEFIR_BIGINT_WIDTH_T
#define __KEFIR_BIGINT_WIDTH_T unsigned long long
#endif

#define __KEFIR_BIGINT_VALUE_BIT (sizeof(__KEFIR_BIGINT_SIGNED_VALUE_T) * __KEFIR_BIGINT_CHAR_BIT)
#define __KEFIR_BIGINT_DIGIT_BIT (sizeof(__KEFIR_BIGINT_DIGIT_T) * __KEFIR_BIGINT_CHAR_BIT)

typedef enum { __KEFIR_BIGINT_OK } __kefir_bigint_result_t;

#define __KEFIR_BIGINT_TRUE 1
#define __KEFIR_BIGINT_FALSE 0
#define __KEFIR_BIGINT_NULL ((void *) 0)

#define __KEFIR_BIGINT_BITS_TO_DIGITS(_bits) (((_bits) + __KEFIR_BIGINT_DIGIT_BIT - 1) / __KEFIR_BIGINT_DIGIT_BIT)

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

static __kefir_bigint_result_t __kefir_bigint_get_signed_value(const unsigned char *digits,
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

static __kefir_bigint_result_t __kefir_bigint_resize_cast_signed(__KEFIR_BIGINT_DIGIT_T *digits,
                                                                 __KEFIR_BIGINT_WIDTH_T current_width,
                                                                 __KEFIR_BIGINT_WIDTH_T desired_width) {
    const __KEFIR_BIGINT_WIDTH_T desired_msb_location = desired_width - 1;
    const __KEFIR_BIGINT_WIDTH_T desired_msb_digit_index = desired_msb_location / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T desired_msb_bit_offset =
        desired_msb_location - desired_msb_digit_index * __KEFIR_BIGINT_DIGIT_BIT;

    if (desired_width <= current_width) {
        __KEFIR_BIGINT_UINT_T desired_msb = (digits[desired_msb_digit_index] >> desired_msb_bit_offset) & 1;
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

    const __KEFIR_BIGINT_WIDTH_T current_msb_location = current_width - 1;
    const __KEFIR_BIGINT_WIDTH_T current_msb_digit_index = current_msb_location / __KEFIR_BIGINT_DIGIT_BIT;
    const __KEFIR_BIGINT_WIDTH_T current_msb_bit_offset =
        current_msb_location - current_msb_digit_index * __KEFIR_BIGINT_DIGIT_BIT;

    __KEFIR_BIGINT_UINT_T current_msb = (digits[current_msb_digit_index] >> current_msb_bit_offset) & 1;
    if (current_msb_bit_offset + 1 < __KEFIR_BIGINT_DIGIT_BIT) {
        if (current_msb) {
            const unsigned long long mask = ~((1ull << current_msb_bit_offset) - 1);
            digits[current_msb_digit_index] |= mask;
        } else {
            const unsigned long long mask = (1ull << current_msb_bit_offset) - 1;
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
        (void) __kefir_bigint_resize_cast_signed(digits, fit_width, width);
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_add(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                  const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                  __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    __KEFIR_BIGINT_UINT_T carry = 0;
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_UINT_T lhs_digit = lhs_digits[i];
        const __KEFIR_BIGINT_UINT_T rhs_digit = rhs_digits[i];

        const __KEFIR_BIGINT_UINT_T digit_sum = carry + lhs_digit + rhs_digit;
        lhs_digits[i] = (__KEFIR_BIGINT_DIGIT_T) digit_sum;
        carry = digit_sum >> __KEFIR_BIGINT_DIGIT_BIT;
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_invert(__KEFIR_BIGINT_DIGIT_T *digits, __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        digits[i] = ~digits[i];
    }
    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_and(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                  const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                  __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_UINT_T lhs_digit = lhs_digits[i];
        const __KEFIR_BIGINT_UINT_T rhs_digit = rhs_digits[i];

        lhs_digits[i] = lhs_digit & rhs_digit;
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_or(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                 const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                 __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_UINT_T lhs_digit = lhs_digits[i];
        const __KEFIR_BIGINT_UINT_T rhs_digit = rhs_digits[i];

        lhs_digits[i] = lhs_digit | rhs_digit;
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_xor(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                  const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                  __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_UINT_T lhs_digit = lhs_digits[i];
        const __KEFIR_BIGINT_UINT_T rhs_digit = rhs_digits[i];

        lhs_digits[i] = lhs_digit ^ rhs_digit;
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_util_add_digit(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                             __KEFIR_BIGINT_DIGIT_T rhs_digit,
                                                             __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);
    const __KEFIR_BIGINT_UINT_T rhs_sign = (rhs_digit >> (__KEFIR_BIGINT_DIGIT_BIT - 1)) & 1;
    const __KEFIR_BIGINT_UINT_T rhs_extension = rhs_sign ? ~(__KEFIR_BIGINT_DIGIT_T) 0 : (__KEFIR_BIGINT_DIGIT_T) 0;

    __KEFIR_BIGINT_UINT_T carry = 0;
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_UINT_T lhs_digit = lhs_digits[i];
        const __KEFIR_BIGINT_UINT_T digit = i == 0 ? rhs_digit : rhs_extension;

        const __KEFIR_BIGINT_UINT_T digit_sum = carry + lhs_digit + digit;
        lhs_digits[i] = (__KEFIR_BIGINT_DIGIT_T) digit_sum;
        carry = digit_sum >> __KEFIR_BIGINT_DIGIT_BIT;
    }

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_negate(__KEFIR_BIGINT_DIGIT_T *digits, __KEFIR_BIGINT_WIDTH_T width) {
    (void) __kefir_bigint_invert(digits, width);
    (void) __kefir_bigint_util_add_digit(digits, 1, width);

    return __KEFIR_BIGINT_OK;
}

static __kefir_bigint_result_t __kefir_bigint_subtract(__KEFIR_BIGINT_DIGIT_T *lhs_digits,
                                                       const __KEFIR_BIGINT_DIGIT_T *rhs_digits,
                                                       __KEFIR_BIGINT_WIDTH_T width) {
    const __KEFIR_BIGINT_WIDTH_T total_digits = __KEFIR_BIGINT_BITS_TO_DIGITS(width);

    __KEFIR_BIGINT_UINT_T carry = 0;
    for (__KEFIR_BIGINT_WIDTH_T i = 0; i < total_digits; i++) {
        const __KEFIR_BIGINT_UINT_T lhs_digit = lhs_digits[i];
        const __KEFIR_BIGINT_UINT_T rhs_digit = rhs_digits[i];

        const __KEFIR_BIGINT_UINT_T digit_diff = ((__KEFIR_BIGINT_UINT_T) lhs_digit) - rhs_digit - carry;
        lhs_digits[i] = (__KEFIR_BIGINT_DIGIT_T) digit_diff;
        carry = (digit_diff >> __KEFIR_BIGINT_DIGIT_BIT) & 1;
    }

    return __KEFIR_BIGINT_OK;
}

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

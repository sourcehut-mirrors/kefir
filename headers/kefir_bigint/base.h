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

#ifndef __KEFIR_BIGINT_BASE_H__
#define __KEFIR_BIGINT_BASE_H__

#ifndef __KEFIR_BIGINT_USE_BIGINT_IMPL__
#error "bigint_impl.h header shall not be included directly"
#endif

#ifndef __KEFIR_BIGINT_CHAR_BIT
#error "bigint_impl.h environment is missing __KEFIR_BIGINT_CHAR_BIT definition"
#endif

#ifndef __KEFIR_BIGINT_FLT_MANT_DIG
#error "bigint_impl.h environment is missing __KEFIR_BIGINT_FLT_MANT_DIG definition"
#endif

#ifndef __KEFIR_BIGINT_DBL_MANT_DIG
#error "bigint_impl.h environment is missing __KEFIR_BIGINT_DBL_MANT_DIG definition"
#endif

#ifndef __KEFIR_BIGINT_UCHAR_T
#define __KEFIR_BIGINT_UCHAR_T unsigned char
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

#ifndef __KEFIR_BIGINT_FLOAT_T
#define __KEFIR_BIGINT_FLOAT_T float
#endif

#ifndef __KEFIR_BIGINT_DOUBLE_T
#define __KEFIR_BIGINT_DOUBLE_T double
#endif

#define __KEFIR_BIGINT_VALUE_BIT (sizeof(__KEFIR_BIGINT_SIGNED_VALUE_T) * __KEFIR_BIGINT_CHAR_BIT)
#define __KEFIR_BIGINT_DIGIT_BIT (sizeof(__KEFIR_BIGINT_DIGIT_T) * __KEFIR_BIGINT_CHAR_BIT)

typedef enum { __KEFIR_BIGINT_OK, __KEFIR_BIGINT_DIVISION_BY_ZERO } __kefir_bigint_result_t;

#define __KEFIR_BIGINT_TRUE 1
#define __KEFIR_BIGINT_FALSE 0
#define __KEFIR_BIGINT_NULL ((void *) 0)

#define __KEFIR_BIGINT_BITS_TO_DIGITS(_bits) (((_bits) + __KEFIR_BIGINT_DIGIT_BIT - 1) / __KEFIR_BIGINT_DIGIT_BIT)
static __KEFIR_BIGINT_WIDTH_T __kefir_bigint_native_signed_width(__KEFIR_BIGINT_SIGNED_VALUE_T);
static __KEFIR_BIGINT_WIDTH_T __kefir_bigint_native_unsigned_width(__KEFIR_BIGINT_UNSIGNED_VALUE_T);
static __kefir_bigint_result_t __kefir_bigint_get_signed_value(const unsigned char *, __KEFIR_BIGINT_WIDTH_T,
                                                               __KEFIR_BIGINT_SIGNED_VALUE_T *);
static __kefir_bigint_result_t __kefir_bigint_get_unsigned_value(const unsigned char *, __KEFIR_BIGINT_WIDTH_T,
                                                                 __KEFIR_BIGINT_UNSIGNED_VALUE_T *);
static __KEFIR_BIGINT_UINT_T __kefir_bigint_get_sign(const __KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T);
static __KEFIR_BIGINT_UINT_T __kefir_bigint_is_zero(const __KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_set_bits(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_UNSIGNED_VALUE_T,
                                                       __KEFIR_BIGINT_WIDTH_T, __KEFIR_BIGINT_WIDTH_T,
                                                       __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_cast_signed(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T,
                                                          __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_cast_unsigned(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T,
                                                            __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_set_signed_integer(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T,
                                                                 __KEFIR_BIGINT_UNSIGNED_VALUE_T);
static __kefir_bigint_result_t __kefir_bigint_set_unsigned_integer(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T,
                                                                   __KEFIR_BIGINT_UNSIGNED_VALUE_T);

static __kefir_bigint_result_t __kefir_bigint_util_add_digit_zero_extended(__KEFIR_BIGINT_DIGIT_T *,
                                                                           __KEFIR_BIGINT_DIGIT_T,
                                                                           __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_add(__KEFIR_BIGINT_DIGIT_T *, const __KEFIR_BIGINT_DIGIT_T *,
                                                  __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_add_zero_extend(__KEFIR_BIGINT_DIGIT_T *, const __KEFIR_BIGINT_DIGIT_T *,
                                                              __KEFIR_BIGINT_WIDTH_T, __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_subtract(__KEFIR_BIGINT_DIGIT_T *, const __KEFIR_BIGINT_DIGIT_T *,
                                                       __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_subtract_zero_extend(__KEFIR_BIGINT_DIGIT_T *,
                                                                   const __KEFIR_BIGINT_DIGIT_T *,
                                                                   __KEFIR_BIGINT_WIDTH_T, __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_or(__KEFIR_BIGINT_DIGIT_T *, const __KEFIR_BIGINT_DIGIT_T *,
                                                 __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_and(__KEFIR_BIGINT_DIGIT_T *, const __KEFIR_BIGINT_DIGIT_T *,
                                                  __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_xor(__KEFIR_BIGINT_DIGIT_T *, const __KEFIR_BIGINT_DIGIT_T *,
                                                  __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_left_shift(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T,
                                                         __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_right_shift(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T,
                                                          __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_arithmetic_right_shift(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T,
                                                                     __KEFIR_BIGINT_WIDTH_T);

static __kefir_bigint_result_t __kefir_bigint_left_shift_whole_digits(__KEFIR_BIGINT_DIGIT_T *,
                                                                      __KEFIR_BIGINT_UNSIGNED_VALUE_T,
                                                                      __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_right_shift_whole_digits(__KEFIR_BIGINT_DIGIT_T *,
                                                                       __KEFIR_BIGINT_UNSIGNED_VALUE_T,
                                                                       __KEFIR_BIGINT_UINT_T, __KEFIR_BIGINT_WIDTH_T);

static __kefir_bigint_result_t __kefir_bigint_zero(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T);

static __kefir_bigint_result_t __kefir_bigint_copy(__KEFIR_BIGINT_DIGIT_T *, const __KEFIR_BIGINT_DIGIT_T *,
                                                   __KEFIR_BIGINT_WIDTH_T);

static __kefir_bigint_result_t __kefir_bigint_unsigned_multiply(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_DIGIT_T *,
                                                                const __KEFIR_BIGINT_DIGIT_T *,
                                                                const __KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T,
                                                                __KEFIR_BIGINT_WIDTH_T);

static __kefir_bigint_result_t __kefir_bigint_signed_multiply(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_DIGIT_T *,
                                                              __KEFIR_BIGINT_DIGIT_T *, const __KEFIR_BIGINT_DIGIT_T *,
                                                              __KEFIR_BIGINT_WIDTH_T, __KEFIR_BIGINT_WIDTH_T);

static __kefir_bigint_result_t __kefir_bigint_unsigned_divide(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_DIGIT_T *,
                                                              const __KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T,
                                                              __KEFIR_BIGINT_WIDTH_T);
static __kefir_bigint_result_t __kefir_bigint_signed_divide(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_DIGIT_T *,
                                                            __KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T,
                                                            __KEFIR_BIGINT_WIDTH_T);
static __KEFIR_BIGINT_UINT_T __kefir_bigint_unsigned_compare(const __KEFIR_BIGINT_DIGIT_T *,
                                                             const __KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T);
static __KEFIR_BIGINT_UINT_T __kefir_bigint_signed_compare(const __KEFIR_BIGINT_DIGIT_T *,
                                                           const __KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_WIDTH_T);

static __KEFIR_BIGINT_FLOAT_T __kefir_bigint_signed_to_float(__KEFIR_BIGINT_DIGIT_T *, __KEFIR_BIGINT_DIGIT_T *,
                                                             __KEFIR_BIGINT_WIDTH_T);

#endif

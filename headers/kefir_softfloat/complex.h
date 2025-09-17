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

#ifndef __KEFIR_SOFTFLOAT_COMPLEX_H__
#define __KEFIR_SOFTFLOAT_COMPLEX_H__

#include "kefir_softfloat/base.h"

// Complex long double multiplication and vision routines provided below are
// based on the algorithms described in the appendix G of
// https://open-std.org/JTC1/SC22/WG14/www/docs/n3220.pdf (see page 542).

#define UPDATE_IF_NAN(_var, _copysign)         \
    do {                                       \
        if (__KEFIR_SOFTFLOAT_ISNAN__(_var)) { \
            _var = _copysign(0.0, _var);       \
        }                                      \
    } while (0)

#define COPYSIGN_IF_INF(_var, _copysign)                \
    do {                                                \
        _var = _copysign(_var##_inf ? 1.0 : 0.0, _var); \
    } while (0)

#define SOFTFLOAT_DEFINE_COMPLEX_MUL(_id, _scalar_type, _complex_type, _copysign, _make_complex)               \
    static _complex_type _id(_scalar_type a, _scalar_type b, _scalar_type c, _scalar_type d) {                 \
        _scalar_type ac = a * c, bd = b * d, ad = a * d, bc = b * c, x = ac - bd, y = ad + bc;                 \
        if (__KEFIR_SOFTFLOAT_ISNAN__(x) && __KEFIR_SOFTFLOAT_ISNAN__(y)) {                                    \
            __KEFIR_SOFTFLOAT_BOOL_TYPE_T__ do_recompute = 0;                                                  \
            const __KEFIR_SOFTFLOAT_BOOL_TYPE_T__ a_inf = __KEFIR_SOFTFLOAT_ISINF_SIGN__(a);                   \
            const __KEFIR_SOFTFLOAT_BOOL_TYPE_T__ b_inf = __KEFIR_SOFTFLOAT_ISINF_SIGN__(b);                   \
            const __KEFIR_SOFTFLOAT_BOOL_TYPE_T__ c_inf = __KEFIR_SOFTFLOAT_ISINF_SIGN__(c);                   \
            const __KEFIR_SOFTFLOAT_BOOL_TYPE_T__ d_inf = __KEFIR_SOFTFLOAT_ISINF_SIGN__(d);                   \
            if (a_inf || b_inf) {                                                                              \
                COPYSIGN_IF_INF(a, _copysign);                                                                 \
                COPYSIGN_IF_INF(b, _copysign);                                                                 \
                UPDATE_IF_NAN(c, _copysign);                                                                   \
                UPDATE_IF_NAN(d, _copysign);                                                                   \
                do_recompute = 1;                                                                              \
            }                                                                                                  \
                                                                                                               \
            if (c_inf || d_inf) {                                                                              \
                COPYSIGN_IF_INF(c, _copysign);                                                                 \
                COPYSIGN_IF_INF(d, _copysign);                                                                 \
                UPDATE_IF_NAN(a, _copysign);                                                                   \
                UPDATE_IF_NAN(b, _copysign);                                                                   \
                do_recompute = 1;                                                                              \
            }                                                                                                  \
                                                                                                               \
            if (!do_recompute && (__KEFIR_SOFTFLOAT_ISINF_SIGN__(ac) || __KEFIR_SOFTFLOAT_ISINF_SIGN__(bd) ||  \
                                  __KEFIR_SOFTFLOAT_ISINF_SIGN__(ad) || __KEFIR_SOFTFLOAT_ISINF_SIGN__(bc))) { \
                UPDATE_IF_NAN(a, _copysign);                                                                   \
                UPDATE_IF_NAN(b, _copysign);                                                                   \
                UPDATE_IF_NAN(c, _copysign);                                                                   \
                UPDATE_IF_NAN(d, _copysign);                                                                   \
                do_recompute = 1;                                                                              \
            }                                                                                                  \
                                                                                                               \
            if (do_recompute) {                                                                                \
                x = __KEFIR_SOFTFLOAT_INFINITY__ * (a * c - b * d);                                            \
                y = __KEFIR_SOFTFLOAT_INFINITY__ * (a * d + b * c);                                            \
            }                                                                                                  \
        }                                                                                                      \
                                                                                                               \
        return _make_complex(x, y);                                                                            \
    }

SOFTFLOAT_DEFINE_COMPLEX_MUL(__kefir_softfloat_complex_float_mul, __KEFIR_SOFTFLOAT_FLOAT_T__,
                             __KEFIR_SOFTFLOAT_COMPLEX_FLOAT_T__, __KEFIR_SOFTFLOAT_COPYSIGNF__,
                             __KEFIR_SOFTFLOAT_MAKE_COMPLEX_FLOAT__)
SOFTFLOAT_DEFINE_COMPLEX_MUL(__kefir_softfloat_complex_double_mul, __KEFIR_SOFTFLOAT_DOUBLE_T__,
                             __KEFIR_SOFTFLOAT_COMPLEX_DOUBLE_T__, __KEFIR_SOFTFLOAT_COPYSIGN__,
                             __KEFIR_SOFTFLOAT_MAKE_COMPLEX_DOUBLE__)
SOFTFLOAT_DEFINE_COMPLEX_MUL(__kefir_softfloat_complex_long_double_mul, __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__,
                             __KEFIR_SOFTFLOAT_COMPLEX_LONG_DOUBLE_T__, __KEFIR_SOFTFLOAT_COPYSIGNL__,
                             __KEFIR_SOFTFLOAT_MAKE_COMPLEX_LONG_DOUBLE__)

#undef SOFTFLOAT_DEFINE_COMPLEX_MUL
#undef COPYSIGN_IF_INF
#undef UPDATE_IF_NAN

#endif

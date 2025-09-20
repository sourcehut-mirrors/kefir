/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kefir/util/softfloat.h"
#include "kefir/core/error.h"
#include "kefir/core/util.h"

typedef struct kefir_softfloat_complex_float {
    kefir_float32_t real;
    kefir_float32_t imaginary;
} kefir_softfloat_complex_float_t;

typedef struct kefir_softfloat_complex_double {
    kefir_float64_t real;
    kefir_float64_t imaginary;
} kefir_softfloat_complex_double_t;

#define MAKE_COMPLEX_FLOAT(_x, _y)           \
    (struct kefir_softfloat_complex_float) { \
        (_x), (_y)                           \
    }
#define MAKE_COMPLEX_DOUBLE(_x, _y)           \
    (struct kefir_softfloat_complex_double) { \
        (_x), (_y)                            \
    }
#define MAKE_COMPLEX_LONG_DOUBLE(_x, _y)           \
    (struct kefir_softfloat_complex_long_double) { \
        (_x), (_y)                                 \
    }

#include <math.h>
#include <float.h>
#define __KEFIR_SOFTFLOAT_USE_SOFTFLOAT_IMPL__
#define __KEFIR_SOFTFLOAT_LDBL_MANT_DIG__ LDBL_MANT_DIG
#define __KEFIR_SOFTFLOAT_INT_MAX__ INT_MAX
#define __KEFIR_SOFTFLOAT_BOOL_TYPE_T__ kefir_bool_t
#define __KEFIR_SOFTFLOAT_INT_T__ kefir_int_t
#define __KEFIR_SOFTFLOAT_UINT64_T__ kefir_uint64_t
#define __KEFIR_SOFTFLOAT_FLOAT_T__ kefir_float32_t
#define __KEFIR_SOFTFLOAT_DOUBLE_T__ kefir_float64_t
#define __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ kefir_long_double_t
#define __KEFIR_SOFTFLOAT_COMPLEX_FLOAT_T__ struct kefir_softfloat_complex_float
#define __KEFIR_SOFTFLOAT_COMPLEX_DOUBLE_T__ struct kefir_softfloat_complex_double
#define __KEFIR_SOFTFLOAT_COMPLEX_LONG_DOUBLE_T__ struct kefir_softfloat_complex_long_double
#define __KEFIR_SOFTFLOAT_ISNAN__ isnan
#define __KEFIR_SOFTFLOAT_ISINF_SIGN__ isinf
#define __KEFIR_SOFTFLOAT_COPYSIGNF__ copysignf
#define __KEFIR_SOFTFLOAT_COPYSIGN__ copysign
#define __KEFIR_SOFTFLOAT_COPYSIGNL__ copysignl
#define __KEFIR_SOFTFLOAT_INFINITY__ INFINITY
#define __KEFIR_SOFTFLOAT_ISGREATER__ isgreater
#define __KEFIR_SOFTFLOAT_ISLESS__ isless
#define __KEFIR_SOFTFLOAT_ISFINITE__ isfinite
#define __KEFIR_SOFTFLOAT_MAKE_COMPLEX_FLOAT__ MAKE_COMPLEX_FLOAT
#define __KEFIR_SOFTFLOAT_MAKE_COMPLEX_DOUBLE__ MAKE_COMPLEX_DOUBLE
#define __KEFIR_SOFTFLOAT_MAKE_COMPLEX_LONG_DOUBLE__ MAKE_COMPLEX_LONG_DOUBLE
#include "kefir_softfloat/softfloat.h"

// Complex long double multiplication and vision routines provided below are
// based on the algorithms described in the appendix G of
// https://open-std.org/JTC1/SC22/WG14/www/docs/n3220.pdf (see page 542).

struct kefir_softfloat_complex_long_double kefir_softfloat_complex_long_double_mul(
    struct kefir_softfloat_complex_long_double lhs, struct kefir_softfloat_complex_long_double rhs) {
    UNUSED(__kefir_softfloat_complex_float_mul);   // Dummy UNUSED -- to avoid compiler complaints
    UNUSED(__kefir_softfloat_complex_double_mul);  // Dummy UNUSED -- to avoid compiler complaints
    return __kefir_softfloat_complex_long_double_mul(lhs.real, lhs.imaginary, rhs.real, rhs.imaginary);
}

struct kefir_softfloat_complex_long_double kefir_softfloat_complex_long_double_div(
    struct kefir_softfloat_complex_long_double lhs, struct kefir_softfloat_complex_long_double rhs) {
    __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ a = lhs.real, b = lhs.imaginary;
    __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ c = rhs.real, d = rhs.imaginary;

    const __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ logbw = __kefir_softfloat_logbl(__kefir_softfloat_fmaximum_numl(__kefir_softfloat_fabsl(c), __kefir_softfloat_fabsl(d)));
    int ilogbw = 0;
    if (__KEFIR_SOFTFLOAT_ISFINITE__(logbw)) {
        ilogbw = (int) logbw;
        c = __kefir_softfloat_scalbnl(c, -ilogbw);
        d = __kefir_softfloat_scalbnl(d, -ilogbw);
    }

    const __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ denom = c * c + d * d;
#define EVAL_X() (a * c + b * d)
#define EVAL_Y() (b * c - a * d)
#define COPYSIGN_IF_INF(_var)                                     \
    do {                                                          \
        *(_var) = __KEFIR_SOFTFLOAT_COPYSIGNL__(__KEFIR_SOFTFLOAT_ISINF_SIGN__(*(_var)) ? 1.0 : 0.0, *(_var)); \
    } while (0)
    __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ x = __kefir_softfloat_scalbnl(EVAL_X() / denom, -ilogbw);
    __KEFIR_SOFTFLOAT_LONG_DOUBLE_T__ y = __kefir_softfloat_scalbnl(EVAL_Y() / denom, -ilogbw);
    if (__KEFIR_SOFTFLOAT_ISNAN__(x) && __KEFIR_SOFTFLOAT_ISNAN__(y)) {
        if (denom == 0.0L && (!__KEFIR_SOFTFLOAT_ISNAN__(a) || !__KEFIR_SOFTFLOAT_ISNAN__(b))) {
            x = __KEFIR_SOFTFLOAT_COPYSIGNL__(__KEFIR_SOFTFLOAT_INFINITY__, c) * a;
            y = __KEFIR_SOFTFLOAT_COPYSIGNL__(__KEFIR_SOFTFLOAT_INFINITY__, c) * b;
        } else if ((__KEFIR_SOFTFLOAT_ISINF_SIGN__(a) || __KEFIR_SOFTFLOAT_ISINF_SIGN__(b)) && __KEFIR_SOFTFLOAT_ISFINITE__(c) && __KEFIR_SOFTFLOAT_ISFINITE__(d)) {
            COPYSIGN_IF_INF(&a);
            COPYSIGN_IF_INF(&b);
            x = __KEFIR_SOFTFLOAT_INFINITY__ * EVAL_X();
            y = __KEFIR_SOFTFLOAT_INFINITY__ * EVAL_Y();
        } else if (__KEFIR_SOFTFLOAT_ISINF_SIGN__(logbw) && logbw > 0.0L && __KEFIR_SOFTFLOAT_ISFINITE__(a) && __KEFIR_SOFTFLOAT_ISFINITE__(b)) {
            COPYSIGN_IF_INF(&c);
            COPYSIGN_IF_INF(&d);
            x = 0.0 * EVAL_X();
            y = 0.0 * EVAL_Y();
        }
#undef COPYSIGN_IF_INF
#undef EVAL_X
#undef EVAL_Y
    }
    return (struct kefir_softfloat_complex_long_double) {x, y};
}

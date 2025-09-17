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
#define __KEFIR_SOFTFLOAT_USE_SOFTFLOAT_IMPL__
#define __KEFIR_SOFTFLOAT_BOOL_TYPE_T__ kefir_bool_t
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
#define __KEFIR_SOFTFLOAT_MAKE_COMPLEX_FLOAT__ MAKE_COMPLEX_FLOAT
#define __KEFIR_SOFTFLOAT_MAKE_COMPLEX_DOUBLE__ MAKE_COMPLEX_DOUBLE
#define __KEFIR_SOFTFLOAT_MAKE_COMPLEX_LONG_DOUBLE__ MAKE_COMPLEX_LONG_DOUBLE
#include "kefir_softfloat/softfloat.h"

// Complex long double multiplication and vision routines provided below are
// based on the algorithms described in the appendix G of
// https://open-std.org/JTC1/SC22/WG14/www/docs/n3220.pdf (see page 542).

static kefir_long_double_t own_fmaximum_numl(kefir_long_double_t x, kefir_long_double_t y) {
    if (isgreater(x, y)) {
        return x;
    } else if (isless(x, y)) {
        return y;
    } else if (x == y) {
        return copysignl(1, x) >= copysignl(1, y) ? x : y;
    } else if (isnan(y)) {
        return isnan(x) ? x + y : x;
    } else {
        return y;
    }
}

struct kefir_softfloat_complex_long_double kefir_softfloat_complex_long_double_mul(
    struct kefir_softfloat_complex_long_double lhs, struct kefir_softfloat_complex_long_double rhs) {
    UNUSED(__kefir_softfloat_complex_float_mul);   // Dummy UNUSED -- to avoid compiler complaints
    UNUSED(__kefir_softfloat_complex_double_mul);  // Dummy UNUSED -- to avoid compiler complaints
    return __kefir_softfloat_complex_long_double_mul(lhs.real, lhs.imaginary, rhs.real, rhs.imaginary);
}

struct kefir_softfloat_complex_long_double kefir_softfloat_complex_long_double_div(
    struct kefir_softfloat_complex_long_double lhs, struct kefir_softfloat_complex_long_double rhs) {
    kefir_long_double_t a = lhs.real, b = lhs.imaginary;
    kefir_long_double_t c = rhs.real, d = rhs.imaginary;

    const kefir_long_double_t logbw = logbl(own_fmaximum_numl(fabsl(c), fabsl(d)));
    int ilogbw = 0;
    if (isfinite(logbw)) {
        ilogbw = (int) logbw;
        c = scalbnl(c, -ilogbw);
        d = scalbnl(d, -ilogbw);
    }

    const kefir_long_double_t denom = c * c + d * d;
#define EVAL_X() (a * c + b * d)
#define EVAL_Y() (b * c - a * d)
#define COPYSIGN_IF_INF(_var)                                     \
    do {                                                          \
        *(_var) = copysignl(isinf(*(_var)) ? 1.0 : 0.0, *(_var)); \
    } while (0)
    kefir_long_double_t x = scalbnl(EVAL_X() / denom, -ilogbw);
    kefir_long_double_t y = scalbnl(EVAL_Y() / denom, -ilogbw);
    if (isnan(x) && isnan(y)) {
        if (denom == 0.0L && (!isnan(a) || !isnan(b))) {
            x = copysignl(INFINITY, c) * a;
            y = copysignl(INFINITY, c) * b;
        } else if ((isinf(a) || isinf(b)) && isfinite(c) && isfinite(d)) {
            COPYSIGN_IF_INF(&a);
            COPYSIGN_IF_INF(&b);
            x = INFINITY * EVAL_X();
            y = INFINITY * EVAL_Y();
        } else if (isinf(logbw) && logbw > 0.0L && isfinite(a) && isfinite(b)) {
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

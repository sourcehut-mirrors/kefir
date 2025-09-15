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
#include <math.h>

// Complex long double multiplication and vision routines provided below are
// based on the algorithms described in the appendix G of
// https://open-std.org/JTC1/SC22/WG14/www/docs/n3220.pdf (see page 542).

static kefir_long_double_t own_fmaximum_numl(kefir_long_double_t x, kefir_long_double_t y) {
    if (isgreater (x, y)) {
        return x;
    } else if (isless (x, y)) {
        return y;
    } else if (x == y) {
        return copysign(1, x) >= copysign(1, y)
            ? x
            : y;
    } else if (isnan (y)) {
        return isnan (x)
            ? x + y
            : x;
    } else {
        return y;
    }
}

struct kefir_softfloat_complex_long_double kefir_softfloat_complex_long_double_mul(struct kefir_softfloat_complex_long_double lhs, struct kefir_softfloat_complex_long_double rhs) {
    kefir_long_double_t a = lhs.real, b = lhs.imaginary,
                        c = rhs.real, d = rhs.imaginary,
                        ac = a * c, bd = b * d,
                        ad = a * d, bc = b * c,
                        x = ac - bd, y = ad + bc;
    if (isnan(x) && isnan(y)) {
        kefir_bool_t do_recompute = false;
        const kefir_bool_t a_inf = isinf(a);
        const kefir_bool_t b_inf = isinf(b);
        const kefir_bool_t c_inf = isinf(c);
        const kefir_bool_t d_inf = isinf(d);
#define UPDATE_IF_NAN(_var) \
        do { \
            if (isnan(_var)) { \
                _var = copysignl(0.0, _var); \
            } \
        } while (0)
#define COPYSIGN_IF_INF(_var) \
        do { \
            _var = copysignl(_var##_inf ? 1.0 : 0.0, _var); \
        } while (0)
        if (a_inf || b_inf) {
            COPYSIGN_IF_INF(a);
            COPYSIGN_IF_INF(b);
            UPDATE_IF_NAN(c);
            UPDATE_IF_NAN(d);
            do_recompute = true;
        }

        if (c_inf || d_inf) {
            COPYSIGN_IF_INF(c);
            COPYSIGN_IF_INF(d);
            UPDATE_IF_NAN(a);
            UPDATE_IF_NAN(b);
            do_recompute = true;
        }

        if (!do_recompute && (isinf(ac) || isinf(bd) || isinf(ad) || isinf(bc))) {
            UPDATE_IF_NAN(a);
            UPDATE_IF_NAN(b);
            UPDATE_IF_NAN(c);
            UPDATE_IF_NAN(d);
            do_recompute = true;
        }
#undef COPYSIGN_IF_INF
#undef UPDATE_IF_NAN

        if (do_recompute) {
            x = INFINITY * (a * c - b * d);
            y = INFINITY * (a * d + b * c);
        }
    }
    return (struct kefir_softfloat_complex_long_double) {
        x, y
    };
}

struct kefir_softfloat_complex_long_double kefir_softfloat_complex_long_double_div(struct kefir_softfloat_complex_long_double lhs, struct kefir_softfloat_complex_long_double rhs) {
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
#define COPYSIGN_IF_INF(_var) \
    do { \
        *(_var) = copysignl(isinf(*(_var)) ? 1.0: 0.0, *(_var)); \
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
    return (struct kefir_softfloat_complex_long_double) {
        x, y
    };
}

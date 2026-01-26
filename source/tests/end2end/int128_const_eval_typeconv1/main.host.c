/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex.h>
#include "./definitions.h"

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && \
    !defined(__NetBSD__) && !defined(__DragonFly__) &&                                                               \
    (__GNUC__ >= 14 || defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)) && !defined(KEFIR_PLATFORM_DECIMAL_DPD)
#pragma GCC diagnostic ignored "-Wpedantic"
#define ENABLE_DECIMAL_TEST
_Bool decimal32_eq(_Decimal32 a, _Decimal32 b) {
    _Decimal32 diff = (a - b) * 10000;
    return diff < 1 && diff > -1;
}

_Bool decimal64_eq(_Decimal64 a, _Decimal64 b) {
    _Decimal64 diff = (a - b) * 100000;
    return diff < 1 && diff > -1;
}

_Bool decimal128_eq(_Decimal128 a, _Decimal128 b) {
    _Decimal128 diff = (a - b) * 1000000;
    return diff < 1 && diff > -1;
}

extern _Decimal32 l[];
extern _Decimal64 m[];
extern _Decimal128 n[];

#endif

int main(void) {
    assert(a[0] == 1029);
    assert(a[1] == -99182);
    assert(a[2] == 991918);
    assert(a[3] == 4276685308);

    assert(b[0] == 1029);
    assert(b[1] == (unsigned long) -99182);
    assert(b[2] == 991918);
    assert(b[3] == 4276685308);

    assert(fabs(c[0] - 1029) < 1e-6);
    assert(fabs(c[1] + 99182) < 1e-6);
    assert(fabs(c[2] - 991918) < 1e-6);
    assert(fabs(c[3] - 4276685312) < 1e-6);

    assert(fabs(d[0] - 1029) < 1e-8);
    assert(fabs(d[1] + 99182) < 1e-8);
    assert(fabs(d[2] - 991918) < 1e-8);
    assert(fabs(d[3] - 4276685308) < 1e-8);

    assert(fabsl(e[0] - 1029) < 1e-9L);
    assert(fabsl(e[1] + 99182) < 1e-9L);
    assert(fabsl(e[2] - 991918) < 1e-9L);
    assert(fabsl(e[3] - 4276685308) < 1e-9L);

    assert(fabs(creal(f[0]) - 1029) < 1e-6);
    assert(fabs(creal(f[1]) + 99182) < 1e-6);
    assert(fabs(creal(f[2]) - 991918) < 1e-6);
    assert(fabs(creal(f[3]) - 4276685312) < 1e-6);

    assert(fabs(creal(g[0]) - 1029) < 1e-8);
    assert(fabs(creal(g[1]) + 99182) < 1e-8);
    assert(fabs(creal(g[2]) - 991918) < 1e-8);
    assert(fabs(creal(g[3]) - 4276685308) < 1e-8);

    assert(fabsl(creall(h[0]) - 1029) < 1e-9L);
    assert(fabsl(creall(h[1]) + 99182) < 1e-9L);
    assert(fabsl(creall(h[2]) - 991918) < 1e-9L);
    assert(fabsl(creall(h[3]) - 4276685308) < 1e-9L);

    assert(i[0] == &h[2]);
    assert(i[1] == &h[3]);

    assert(j[0].arr[0] == 9024705113554399975ull);
    assert(j[0].arr[1] == (unsigned long) -5582662ll);
    assert(j[0].arr[2] == ~0ull);
    assert(j[0].arr[3] == ~0ull);
    assert(j[1].arr[0] == (unsigned long) -7099464801848784615ll);
    assert(j[1].arr[1] == (unsigned long) 55826);
    assert(j[1].arr[2] == 0);
    assert(j[1].arr[3] == 0);

    assert(k[0].arr[0] == 9024705113554399975ull);
    assert(k[0].arr[1] == (unsigned long) -5582662ll);
    assert(k[0].arr[2] == ~0ull);
    assert(k[0].arr[3] == ~0ull);
    assert(k[1].arr[0] == (unsigned long) -7099464801848784615ll);
    assert(k[1].arr[1] == (unsigned long) 55826);
    assert(k[1].arr[2] == 0);
    assert(k[1].arr[3] == 0);

#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(l[0], -3813918391));
    assert(decimal32_eq(l[1], 83818381993819304ull));

    assert(decimal64_eq(m[0], -3813918391));
    assert(decimal64_eq(m[1], 83818381993819304ull));

    assert(decimal128_eq(n[0], -3813918391));
    assert(decimal128_eq(n[1], 83818381993819304ull));
#endif
    return EXIT_SUCCESS;
}

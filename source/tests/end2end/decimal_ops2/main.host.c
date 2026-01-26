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
#include "./definitions.h"

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && !defined(__NetBSD__) && !defined(__DragonFly__)
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

_Decimal32 add32(_Decimal32 *, _Decimal32);
_Decimal64 add64(_Decimal64 *, _Decimal64);
_Decimal128 add128(_Decimal128 *, _Decimal128);
_Decimal32 sub32(_Decimal32 *, _Decimal32);
_Decimal64 sub64(_Decimal64 *, _Decimal64);
_Decimal128 sub128(_Decimal128 *, _Decimal128);
_Decimal32 mul32(_Decimal32 *, _Decimal32);
_Decimal64 mul64(_Decimal64 *, _Decimal64);
_Decimal128 mul128(_Decimal128 *, _Decimal128);
_Decimal32 div32(_Decimal32 *, _Decimal32);
_Decimal64 div64(_Decimal64 *, _Decimal64);
_Decimal128 div128(_Decimal128 *, _Decimal128);

_Decimal32 preinc32(_Decimal32 *);
_Decimal64 preinc64(_Decimal64 *);
_Decimal128 preinc128(_Decimal128 *);
_Decimal32 postinc32(_Decimal32 *);
_Decimal64 postinc64(_Decimal64 *);
_Decimal128 postinc128(_Decimal128 *);
_Decimal32 predec32(_Decimal32 *);
_Decimal64 predec64(_Decimal64 *);
_Decimal128 predec128(_Decimal128 *);
_Decimal32 postdec32(_Decimal32 *);
_Decimal64 postdec64(_Decimal64 *);
_Decimal128 postdec128(_Decimal128 *);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    _Decimal32 d32 = 3.14159;
    _Decimal64 d64 = 3.14159;
    _Decimal128 d128 = 3.14159;
    assert(decimal32_eq(add32(&d32, 2.71828), 3.14159 + 2.71828));
    assert(decimal32_eq(d32, 3.14159 + 2.71828));
    assert(decimal64_eq(add64(&d64, 2.71828), 3.14159 + 2.71828));
    assert(decimal64_eq(d64, 3.14159 + 2.71828));
    assert(decimal128_eq(add128(&d128, 2.71828), 3.14159 + 2.71828));
    assert(decimal128_eq(d128, 3.14159 + 2.71828));

    d32 = 3.14159;
    d64 = 3.14159;
    d128 = 3.14159;
    assert(decimal32_eq(sub32(&d32, 2.71828), 3.14159 - 2.71828));
    assert(decimal32_eq(d32, 3.14159 - 2.71828));
    assert(decimal64_eq(sub64(&d64, 2.71828), 3.14159 - 2.71828));
    assert(decimal64_eq(d64, 3.14159 - 2.71828));
    assert(decimal128_eq(sub128(&d128, 2.71828), 3.14159 - 2.71828));
    assert(decimal128_eq(d128, 3.14159 - 2.71828));

    d32 = 3.14159;
    d64 = 3.14159;
    d128 = 3.14159;
    assert(decimal32_eq(mul32(&d32, 2.71828), 3.14159 * 2.71828));
    assert(decimal32_eq(d32, 3.14159 * 2.71828));
    assert(decimal64_eq(mul64(&d64, 2.71828), 3.14159 * 2.71828));
    assert(decimal64_eq(d64, 3.14159 * 2.71828));
    assert(decimal128_eq(mul128(&d128, 2.71828), 3.14159 * 2.71828));
    assert(decimal128_eq(d128, 3.14159 * 2.71828));

    d32 = 3.14159;
    d64 = 3.14159;
    d128 = 3.14159;
    assert(decimal32_eq(div32(&d32, 2.71828), 3.14159 / 2.71828));
    assert(decimal32_eq(d32, 3.14159 / 2.71828));
    assert(decimal64_eq(div64(&d64, 2.71828), 3.14159 / 2.71828));
    assert(decimal64_eq(d64, 3.14159 / 2.71828));
    assert(decimal128_eq(div128(&d128, 2.71828), 3.14159 / 2.71828));
    assert(decimal128_eq(d128, 3.14159 / 2.71828));

    d32 = 3.14159;
    d64 = 3.14159;
    d128 = 3.14159;
    assert(decimal32_eq(preinc32(&d32), 3.14159 + 1));
    assert(decimal32_eq(d32, 3.14159 + 1));
    assert(decimal64_eq(preinc64(&d64), 3.14159 + 1));
    assert(decimal64_eq(d64, 3.14159 + 1));
    assert(decimal128_eq(preinc128(&d128), 3.14159 + 1));
    assert(decimal128_eq(d128, 3.14159 + 1));

    d32 = 3.14159;
    d64 = 3.14159;
    d128 = 3.14159;
    assert(decimal32_eq(predec32(&d32), 3.14159 - 1));
    assert(decimal32_eq(d32, 3.14159 - 1));
    assert(decimal64_eq(predec64(&d64), 3.14159 - 1));
    assert(decimal64_eq(d64, 3.14159 - 1));
    assert(decimal128_eq(predec128(&d128), 3.14159 - 1));
    assert(decimal128_eq(d128, 3.14159 - 1));

    d32 = 3.14159;
    d64 = 3.14159;
    d128 = 3.14159;
    assert(decimal32_eq(postinc32(&d32), 3.14159));
    assert(decimal32_eq(d32, 3.14159 + 1));
    assert(decimal64_eq(postinc64(&d64), 3.14159));
    assert(decimal64_eq(d64, 3.14159 + 1));
    assert(decimal128_eq(postinc128(&d128), 3.14159));
    assert(decimal128_eq(d128, 3.14159 + 1));

    d32 = 3.14159;
    d64 = 3.14159;
    d128 = 3.14159;
    assert(decimal32_eq(postdec32(&d32), 3.14159));
    assert(decimal32_eq(d32, 3.14159 - 1));
    assert(decimal64_eq(postdec64(&d64), 3.14159));
    assert(decimal64_eq(d64, 3.14159 - 1));
    assert(decimal128_eq(postdec128(&d128), 3.14159));
    assert(decimal128_eq(d128, 3.14159 - 1));
#endif
    return EXIT_SUCCESS;
}

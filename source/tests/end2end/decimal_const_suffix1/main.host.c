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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "./definitions.h"

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && !defined(__NetBSD__) && !defined(__DragonFly__)
#pragma GCC diagnostic ignored "-Wpedantic"
#define ENABLE_DECIMAL_TEST
_Bool decimal32_eq(_Decimal32 a, _Decimal32 b) {
    _Decimal32 diff = (a - b) * 1000;
    return diff < 1 && diff > -1;
}

_Bool decimal64_eq(_Decimal64 a, _Decimal64 b) {
    _Decimal64 diff = (a - b) * 10000;
    return diff < 1 && diff > -1;
}

_Bool decimal128_eq(_Decimal128 a, _Decimal128 b) {
    _Decimal128 diff = (a - b) * 100000;
    return diff < 1 && diff > -1;
}

extern _Decimal32 a;
extern _Decimal64 b;
extern _Decimal128 c;
extern _Decimal128 d;

_Decimal32 mygeta(void);
_Decimal64 mygetb(void);
_Decimal128 mygetc(void);
_Decimal128 mygetd(void);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(a, 3.14129df));
    assert(decimal64_eq(b, -2.71828dd));
    assert(decimal128_eq(c, 0.319371938e5dl));
    assert(decimal128_eq(d, -7536.4252dl));

    assert(decimal32_eq(mygeta(), 3.14129df));
    assert(decimal64_eq(mygetb(), -2.71828dd));
    assert(decimal64_eq(mygetc(), 0.319371938e5dl));
    assert(decimal128_eq(mygetd(), -7536.4252dl));
#endif
    return EXIT_SUCCESS;
}

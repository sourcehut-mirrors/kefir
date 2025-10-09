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

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && !defined(__NetBSD__)
#pragma GCC diagnostic ignored "-Wpedantic"
#define ENABLE_DECIMAL_TEST
_Bool decimal128_eq(_Decimal128 a, _Decimal128 b) {
    _Decimal128 diff = (a - b) * 100000;
    return diff < 1 && diff > -1;
}

extern int d64x_size;
extern int d64x_alignment;
extern const _Decimal128 *d64x_const_ptr;
extern int d64x_compat[];
extern _Decimal128 d64x[];

_Decimal128 get32x_1(void);
_Decimal128 get32x_2(void);

_Decimal128 neg32x(_Decimal128);
_Decimal128 add32x(_Decimal128, _Decimal128);
_Decimal128 sub32x(_Decimal128, _Decimal128);
_Decimal128 mul32x(_Decimal128, _Decimal128);
_Decimal128 div32x(_Decimal128, _Decimal128);
_Decimal128 conv1(long);
_Decimal128 conv2(unsigned long);
_Decimal128 conv3(float);
_Decimal128 conv4(double);
_Decimal128 conv5(long double);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(d64x_size == sizeof(_Decimal128));
    assert(d64x_alignment == _Alignof(_Decimal128));
    assert(decimal128_eq(*d64x_const_ptr, 2.71828f));
    assert(d64x_compat[0] == -1);
    assert(d64x_compat[1] == -1);
    assert(d64x_compat[2] == -1);
    assert(d64x_compat[3] == -1);
    assert(d64x_compat[4] == 6);
    assert(d64x_compat[5] == -1);
    assert(d64x_compat[6] == -1);
    assert(d64x_compat[7] == 6);

    int i = 0;
    assert(decimal128_eq(d64x[i++], ((double) 3.14159f)));
    assert(decimal128_eq(d64x[i++], (-((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], (((double) 273.3f) + ((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], (((double) 273.3f) - ((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], (((double) 273.3f) * ((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], (((double) 273.3f) / ((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], ((273.3f) + ((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], ((273.3f) - ((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], ((273.3f) * ((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], ((273.3f) / ((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], (((double) 273.3f) + ((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], (((double) 273.3f) - ((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], (((double) 273.3f) * ((double) 1902.318f))));
    assert(decimal128_eq(d64x[i++], (((double) 273.3f) / ((double) 1902.318f))));

    assert(decimal128_eq(get32x_1(), 5.428f));
    assert(decimal128_eq(get32x_2(), 2.71828f));
    assert(decimal128_eq(neg32x(-42842.31f), 42842.31f));
    assert(decimal128_eq(add32x(0.428f, 1.522f), (0.428f + 1.522f)));
    assert(decimal128_eq(sub32x(0.428f, 1.522f), (0.428f - 1.522f)));
    assert(decimal128_eq(mul32x(0.428f, 1.522f), (0.428f * 1.522f)));
    assert(decimal128_eq(div32x(0.428f, 1.522f), (0.428f / 1.522f)));
    assert(decimal128_eq(conv1(-42849), -42849));
    assert(decimal128_eq(conv2(130015), 130015));
    assert(decimal128_eq(conv3(9.130e1f), 9.130e1));
    assert(decimal128_eq(conv4(9.130e-1), 9.130e-1));
    assert(decimal128_eq(conv5(-1831.41L), -1831.41));
#endif
    return EXIT_SUCCESS;
}

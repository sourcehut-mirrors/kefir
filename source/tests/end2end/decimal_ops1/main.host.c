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

#if defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__) && !defined(__NetBSD__)
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

_Decimal32 add32(_Decimal32, _Decimal32);
_Decimal64 add64(_Decimal64, _Decimal64);
_Decimal128 add128(_Decimal128, _Decimal128);
_Decimal32 sub32(_Decimal32, _Decimal32);
_Decimal64 sub64(_Decimal64, _Decimal64);
_Decimal128 sub128(_Decimal128, _Decimal128);
_Decimal32 mul32(_Decimal32, _Decimal32);
_Decimal64 mul64(_Decimal64, _Decimal64);
_Decimal128 mul128(_Decimal128, _Decimal128);
_Decimal32 div32(_Decimal32, _Decimal32);
_Decimal64 div64(_Decimal64, _Decimal64);
_Decimal128 div128(_Decimal128, _Decimal128);
_Decimal32 neg32(_Decimal32);
_Decimal64 neg64(_Decimal64);
_Decimal128 neg128(_Decimal128);
int eq32(_Decimal32, _Decimal32);
int eq64(_Decimal64, _Decimal64);
int eq128(_Decimal128, _Decimal128);
int gt32(_Decimal32, _Decimal32);
int gt64(_Decimal64, _Decimal64);
int gt128(_Decimal128, _Decimal128);
int lt32(_Decimal32, _Decimal32);
int lt64(_Decimal64, _Decimal64);
int lt128(_Decimal128, _Decimal128);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(add32(3.14159, 2.71828), 3.14159 + 2.71828));
    assert(decimal64_eq(add64(3.14159, 2.71828), 3.14159 + 2.71828));
    assert(decimal128_eq(add128(3.14159, 2.71828), 3.14159 + 2.71828));

    assert(decimal32_eq(sub32(3.14159, 2.71828), 3.14159 - 2.71828));
    assert(decimal64_eq(sub64(3.14159, 2.71828), 3.14159 - 2.71828));
    assert(decimal128_eq(sub128(3.14159, 2.71828), 3.14159 - 2.71828));

    assert(decimal32_eq(mul32(3.14159, 2.71828), 3.14159 * 2.71828));
    assert(decimal64_eq(mul64(3.14159, 2.71828), 3.14159 * 2.71828));
    assert(decimal128_eq(mul128(3.14159, 2.71828), 3.14159 * 2.71828));

    assert(decimal32_eq(div32(3.14159, 2.71828), 3.14159 / 2.71828));
    assert(decimal64_eq(div64(3.14159, 2.71828), 3.14159 / 2.71828));
    assert(decimal128_eq(div128(3.14159, 2.71828), 3.14159 / 2.71828));

    assert(decimal32_eq(neg32(3.14159), -3.14159));
    assert(decimal64_eq(neg64(3.14159), -3.14159));
    assert(decimal128_eq(neg128(3.14159), -3.14159));
    assert(decimal32_eq(neg32(-3.14159), 3.14159));
    assert(decimal64_eq(neg64(-3.14159), 3.14159));
    assert(decimal128_eq(neg128(-3.14159), 3.14159));

    _Decimal32 d32[] = {
        3.14159, 2.71828
    };
    _Decimal64 d64[] = {
        3.14159, 2.71828
    };
    _Decimal128 d128[] = {
        3.14159, 2.71828
    };
    assert(eq32(d32[0], d32[0]));
    assert(eq32(d32[1], d32[1]));
    assert(!eq32(d32[0], d32[1]));
    assert(!eq32(d32[1], d32[0]));

    assert(eq64(d64[0], d64[0]));
    assert(eq64(d64[1], d64[1]));
    assert(!eq64(d64[0], d64[1]));
    assert(!eq64(d64[1], d64[0]));

    assert(eq128(d128[0], d128[0]));
    assert(eq128(d128[1], d128[1]));
    assert(!eq128(d128[0], d128[1]));
    assert(!eq128(d128[1], d128[0]));

    assert(!gt32(d32[0], d32[0]));
    assert(gt32(d32[0], d32[1]));
    assert(!gt32(d32[1], d32[0]));
    assert(gt32(d32[1], -d32[0]));
    assert(!gt32(d32[1], d32[1]));
    assert(gt32(d32[0], -d32[0]));

    assert(!gt64(d64[0], d64[0]));
    assert(gt64(d64[0], d64[1]));
    assert(!gt64(d64[1], d64[0]));
    assert(gt64(d64[1], -d64[0]));
    assert(!gt64(d64[1], d64[1]));
    assert(gt64(d64[0], -d64[0]));

    assert(!gt128(d128[0], d128[0]));
    assert(gt128(d128[0], d128[1]));
    assert(!gt128(d128[1], d128[0]));
    assert(gt128(d128[1], -d128[0]));
    assert(!gt128(d128[1], d128[1]));
    assert(gt128(d128[0], -d128[0]));

    assert(!lt32(d32[0], d32[0]));
    assert(!lt32(d32[0], d32[1]));
    assert(lt32(d32[1], d32[0]));
    assert(!lt32(d32[1], -d32[0]));
    assert(!lt32(d32[1], d32[1]));
    assert(!lt32(d32[0], -d32[0]));
    assert(lt32(-d32[0], d32[1]));
    assert(lt32(-d32[0], -d32[1]));
    assert(lt32(-d32[1], d32[0]));

    assert(!lt64(d64[0], d64[0]));
    assert(!lt64(d64[0], d64[1]));
    assert(lt64(d64[1], d64[0]));
    assert(!lt64(d64[1], -d64[0]));
    assert(!lt64(d64[1], d64[1]));
    assert(!lt64(d64[0], -d64[0]));
    assert(lt64(-d64[0], d64[1]));
    assert(lt64(-d64[0], -d64[1]));
    assert(lt64(-d64[1], d64[0]));

    assert(!lt128(d128[0], d128[0]));
    assert(!lt128(d128[0], d128[1]));
    assert(lt128(d128[1], d128[0]));
    assert(!lt128(d128[1], -d128[0]));
    assert(!lt128(d128[1], d128[1]));
    assert(!lt128(d128[0], -d128[0]));
    assert(lt128(-d128[0], d128[1]));
    assert(lt128(-d128[0], -d128[1]));
    assert(lt128(-d128[1], d128[0]));
#endif
    return EXIT_SUCCESS;
}

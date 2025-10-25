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

_Decimal32 get32(void);
_Decimal64 get64(void);
_Decimal128 get128(void);

_Decimal32 arg32(_Decimal32);
_Decimal64 arg64(_Decimal64);
_Decimal128 arg128(_Decimal128);

_Decimal32 arg32x(int, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32);
_Decimal64 arg64x(int, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64);
_Decimal128 arg128x(int, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128);

struct struct32 {
    _Decimal32 x;
    _Decimal32 y;
    _Decimal32 z;
    _Decimal32 w;
    _Decimal32 a;
    _Decimal32 b;
    _Decimal32 c;
    _Decimal32 d;
    _Decimal32 e;
    _Decimal32 f;
    _Decimal32 g;
    _Decimal32 h;
};

struct struct64 {
    _Decimal64 x;
    _Decimal64 y;
    _Decimal64 z;
    _Decimal64 w;
    _Decimal64 a;
    _Decimal64 b;
    _Decimal64 c;
    _Decimal64 d;
    _Decimal64 e;
    _Decimal64 f;
    _Decimal64 g;
    _Decimal64 h;
};

struct struct128 {
    _Decimal128 x;
    _Decimal128 y;
    _Decimal128 z;
    _Decimal128 w;
    _Decimal128 a;
    _Decimal128 b;
    _Decimal128 c;
    _Decimal128 d;
    _Decimal128 e;
    _Decimal128 f;
    _Decimal128 g;
    _Decimal128 h;
};

_Decimal32 load32(_Decimal32 *);
_Decimal64 load64(_Decimal64 *);
_Decimal128 load128(_Decimal128 *);

void store32(_Decimal32 *, _Decimal32);
void store64(_Decimal64 *, _Decimal64);
void store128(_Decimal128 *, _Decimal128);

struct struct32 ret32(_Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32, _Decimal32);
struct struct64 ret64(_Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64, _Decimal64);
struct struct128 ret128(_Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128, _Decimal128);


struct struct32_2 {
    _Decimal32 a;
    _Decimal32 b;
};

struct struct64_2 {
    _Decimal64 a;
    _Decimal64 b;
};

struct struct128_2 {
    _Decimal128 a;
    _Decimal128 b;
};

struct struct32_2 swap32(struct struct32_2);
struct struct64_2 swap64(struct struct64_2);
struct struct128_2 swap128(struct struct128_2);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(get32(), 30.14159));
    assert(decimal64_eq(get64(), 2274.31884));
    assert(decimal128_eq(get128(), 4818418.471847));

    assert(decimal32_eq(arg32(8427.47), 8427.47));
    assert(decimal64_eq(arg64(84428.47248), 84428.47248));
    assert(decimal128_eq(arg128(10381048.1891), 10381048.1891));

    _Decimal32 args32[] = {
        7371.41,
        -3719.031,
        4920.583,
        8491.42,
        -939.593,
        -59.582,
        495.38,
        -593.852,
        493.102,
        -9310.9,
        9410.18,
        17291.38
    };
    for (unsigned int i = 0; i < sizeof(args32) / sizeof(args32[0]); i++) {
        assert(decimal32_eq(arg32x(i, args32[0], args32[1], args32[2], args32[3], args32[4], args32[5], args32[6], args32[7], args32[8], args32[9], args32[10], args32[11]), args32[i]));
        assert(decimal64_eq(arg64x(i, args32[0], args32[1], args32[2], args32[3], args32[4], args32[5], args32[6], args32[7], args32[8], args32[9], args32[10], args32[11]), args32[i]));
        assert(decimal128_eq(arg128x(i, args32[0], args32[1], args32[2], args32[3], args32[4], args32[5], args32[6], args32[7], args32[8], args32[9], args32[10], args32[11]), args32[i]));

        _Decimal64 clone64 = args32[i];
        _Decimal128 clone128 = args32[i];
        assert(decimal32_eq(load32(&args32[i]), args32[i]));
        assert(decimal64_eq(load64(&clone64), args32[i]));
        assert(decimal128_eq(load128(&clone128), args32[i]));

        _Decimal32 target32;
        _Decimal64 target64;
        _Decimal128 target128;
        store32(&target32, args32[i]);
        store64(&target64, args32[i]);
        store128(&target128, args32[i]);

        assert(decimal32_eq(target32, args32[i]));
        assert(decimal64_eq(target64, args32[i]));
        assert(decimal128_eq(target128, args32[i]));

        struct struct32 s32 = ret32(args32[0], args32[1], args32[2], args32[3], args32[4], args32[5], args32[6], args32[7], args32[8], args32[9], args32[10], args32[11]);
        assert(decimal32_eq(s32.x, args32[0]));
        assert(decimal32_eq(s32.y, args32[1]));
        assert(decimal32_eq(s32.z, args32[2]));
        assert(decimal32_eq(s32.w, args32[3]));
        assert(decimal32_eq(s32.a, args32[4]));
        assert(decimal32_eq(s32.b, args32[5]));
        assert(decimal32_eq(s32.c, args32[6]));
        assert(decimal32_eq(s32.d, args32[7]));
        assert(decimal32_eq(s32.e, args32[8]));
        assert(decimal32_eq(s32.f, args32[9]));
        assert(decimal32_eq(s32.g, args32[10]));
        assert(decimal32_eq(s32.h, args32[11]));

        struct struct64 s64 = ret64(args32[0], args32[1], args32[2], args32[3], args32[4], args32[5], args32[6], args32[7], args32[8], args32[9], args32[10], args32[11]);
        assert(decimal64_eq(s64.x, args32[0]));
        assert(decimal64_eq(s64.y, args32[1]));
        assert(decimal64_eq(s64.z, args32[2]));
        assert(decimal64_eq(s64.w, args32[3]));
        assert(decimal64_eq(s64.a, args32[4]));
        assert(decimal64_eq(s64.b, args32[5]));
        assert(decimal64_eq(s64.c, args32[6]));
        assert(decimal64_eq(s64.d, args32[7]));
        assert(decimal64_eq(s64.e, args32[8]));
        assert(decimal64_eq(s64.f, args32[9]));
        assert(decimal64_eq(s64.g, args32[10]));
        assert(decimal64_eq(s64.h, args32[11]));

        struct struct128 s128 = ret128(args32[0], args32[1], args32[2], args32[3], args32[4], args32[5], args32[6], args32[7], args32[8], args32[9], args32[10], args32[11]);
        assert(decimal128_eq(s128.x, args32[0]));
        assert(decimal128_eq(s128.y, args32[1]));
        assert(decimal128_eq(s128.z, args32[2]));
        assert(decimal128_eq(s128.w, args32[3]));
        assert(decimal128_eq(s128.a, args32[4]));
        assert(decimal128_eq(s128.b, args32[5]));
        assert(decimal128_eq(s128.c, args32[6]));
        assert(decimal128_eq(s128.d, args32[7]));
        assert(decimal128_eq(s128.e, args32[8]));
        assert(decimal128_eq(s128.f, args32[9]));
        assert(decimal128_eq(s128.g, args32[10]));
        assert(decimal128_eq(s128.h, args32[11]));

        struct struct32_2 s32_2 = swap32((struct struct32_2){4482.317, -381.47});
        assert(decimal32_eq(s32_2.a, -381.47));
        assert(decimal32_eq(s32_2.b, 4482.317));

        struct struct64_2 s64_2 = swap64((struct struct64_2){4482.31217, -381.3247});
        assert(decimal64_eq(s64_2.a, -381.3247));
        assert(decimal64_eq(s64_2.b, 4482.31217));

        struct struct128_2 s128_2 = swap128((struct struct128_2){449082.31217, -34281.3247});
        assert(decimal128_eq(s128_2.a, -34281.3247));
        assert(decimal128_eq(s128_2.b, 449082.31217));
    }
#endif
    return EXIT_SUCCESS;
}

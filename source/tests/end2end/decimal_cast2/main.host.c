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

#if defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__) && !defined(__NetBSD__) && __GNUC__ >= 14
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

_Decimal32 int_to_d32(long);
_Decimal32 uint_to_d32(unsigned long);
_Decimal64 int_to_d64(long);
_Decimal64 uint_to_d64(unsigned long);
_Decimal128 int_to_d128(long);
_Decimal128 uint_to_d128(unsigned long);
long d32_to_int(_Decimal32);
unsigned long d32_to_uint(_Decimal32);
long d64_to_int(_Decimal64);
unsigned long d64_to_uint(_Decimal64);
long d128_to_int(_Decimal128);
unsigned long d128_to_uint(_Decimal128);
_Bool d32_to_bool(_Decimal32);
_Bool d64_to_bool(_Decimal64);
_Bool d128_to_bool(_Decimal128);

struct S3 {
    unsigned long arr[3];
};

_Decimal32 bitint24_to_d32(int);
_Decimal32 bitint180_to_d32(struct S3);
_Decimal64 bitint24_to_d64(int);
_Decimal64 bitint180_to_d64(struct S3);
_Decimal128 bitint24_to_d128(int);
_Decimal128 bitint180_to_d128(struct S3);

_Decimal32 ubitint24_to_d32(int);
_Decimal32 ubitint180_to_d32(struct S3);
_Decimal64 ubitint24_to_d64(int);
_Decimal64 ubitint180_to_d64(struct S3);
_Decimal128 ubitint24_to_d128(int);
_Decimal128 ubitint180_to_d128(struct S3);

unsigned int d32_to_bitint24(_Decimal32);
unsigned int d32_to_ubitint24(_Decimal32);
unsigned int d64_to_bitint24(_Decimal64);
unsigned int d64_to_ubitint24(_Decimal64);
unsigned int d128_to_bitint24(_Decimal128);
unsigned int d128_to_ubitint24(_Decimal128);

struct S3 d32_to_bitint180(_Decimal32);
struct S3 d32_to_ubitint180(_Decimal32);
struct S3 d64_to_bitint180(_Decimal64);
struct S3 d64_to_ubitint180(_Decimal64);
struct S3 d128_to_bitint180(_Decimal128);
struct S3 d128_to_ubitint180(_Decimal128);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    assert(decimal32_eq(int_to_d32(-314159), -314159));
    assert(decimal32_eq(uint_to_d32(314159), 314159));
    assert(decimal64_eq(int_to_d64(-314159), -314159));
    assert(decimal64_eq(uint_to_d64(314159), 314159));
    assert(decimal128_eq(int_to_d128(-314159), -314159));
    assert(decimal128_eq(uint_to_d128(314159), 314159));

    assert(d32_to_int(-3.14159e2) == -314);
    assert(d32_to_uint(3.14159e2) == 314);
    assert(d64_to_int(-3.14159e2) == -314);
    assert(d64_to_uint(3.14159e2) == 314);
    assert(d128_to_int(-3.14159e2) == -314);
    assert(d128_to_uint(3.14159e2) == 314);

    assert(d32_to_bool(3.1159));
    assert(d32_to_bool(INFINITY));
    assert(d32_to_bool(nanf("")));
    assert(!d32_to_bool(0.0));

    assert(d64_to_bool(3.1159));
    assert(d64_to_bool(INFINITY));
    assert(d64_to_bool(nanf("")));
    assert(!d64_to_bool(0.0));

    assert(d128_to_bool(3.1159));
    assert(d128_to_bool(INFINITY));
    assert(d128_to_bool(nanf("")));
    assert(!d128_to_bool(0.0));

    assert(decimal32_eq(bitint24_to_d32(10240), 10240));
    assert(decimal32_eq(bitint24_to_d32(-1), -1));
    assert(decimal32_eq(bitint24_to_d32(-3942), -3942));
    assert(decimal32_eq(bitint24_to_d32((1 << 23) - 1), (1 << 23) - 1));
    assert(decimal32_eq(bitint24_to_d32((1 << 23)), -(1 << 23)));
    assert(decimal32_eq(bitint180_to_d32((struct S3){{-1, -1, -1}}), -1));
    assert(decimal32_eq(bitint180_to_d32((struct S3){{0, -1, -1}}), -1 - (double) ~0ull));

    assert(decimal64_eq(bitint24_to_d64(10240), 10240));
    assert(decimal64_eq(bitint24_to_d64(-1), -1));
    assert(decimal64_eq(bitint24_to_d64(-3942), -3942));
    assert(decimal64_eq(bitint24_to_d64((1 << 23) - 1), (1 << 23) - 1));
    assert(decimal64_eq(bitint24_to_d64((1 << 23)), -(1 << 23)));
    assert(decimal64_eq(bitint180_to_d64((struct S3){{-1, -1, -1}}), -1));
    assert(decimal64_eq(bitint180_to_d64((struct S3){{0, -1, -1}}), -1 - (double) ~0ull));

    assert(decimal128_eq(bitint24_to_d128(10240), 10240));
    assert(decimal128_eq(bitint24_to_d128(-1), -1));
    assert(decimal128_eq(bitint24_to_d128(-3942), -3942));
    assert(decimal128_eq(bitint24_to_d128((1 << 23) - 1), (1 << 23) - 1));
    assert(decimal128_eq(bitint24_to_d128((1 << 23)), -(1 << 23)));
    assert(decimal128_eq(bitint180_to_d128((struct S3){{-1, -1, -1}}), -1));

    assert(decimal32_eq(ubitint24_to_d32(10240), 10240));
    assert(decimal32_eq(ubitint24_to_d32(-1), (1 << 24) - 1));
    assert(decimal32_eq(ubitint24_to_d32(-3942), ((unsigned long) -3942) & ((1 << 24) - 1)));
    assert(decimal32_eq(ubitint24_to_d32((1 << 23) - 1), (1 << 23) - 1));
    assert(decimal32_eq(ubitint24_to_d32((1 << 23)), (1 << 23)));
    assert(decimal32_eq(ubitint180_to_d32((struct S3){{0, 1, 0}}), 1 + (double) ~0ull));
    assert(decimal32_eq(ubitint180_to_d32((struct S3){{0, -1, 0}}), ~0ull * (double) ~0ull));
    assert(decimal32_eq(ubitint180_to_d32((struct S3){{0, 0, 1}}), 1 + ~0ull * (double) ~0ull));

    assert(decimal64_eq(ubitint24_to_d64(10240), 10240));
    assert(decimal64_eq(ubitint24_to_d64(-1), (1 << 24) - 1));
    assert(decimal64_eq(ubitint24_to_d64(-3942), ((unsigned long) -3942) & ((1 << 24) - 1)));
    assert(decimal64_eq(ubitint24_to_d64((1 << 23) - 1), (1 << 23) - 1));
    assert(decimal64_eq(ubitint24_to_d64((1 << 23)), (1 << 23)));
    assert(decimal64_eq(ubitint180_to_d64((struct S3){{0, 1, 0}}), 1 + (double) ~0ull));
    assert(decimal64_eq(ubitint180_to_d64((struct S3){{0, -1, 0}}), ~0ull * (double) ~0ull));
    assert(decimal64_eq(ubitint180_to_d64((struct S3){{0, 0, 1}}), 1 + ~0ull * (double) ~0ull));

    assert(decimal128_eq(ubitint24_to_d128(10240), 10240));
    assert(decimal128_eq(ubitint24_to_d128(-1), (1 << 24) - 1));
    assert(decimal128_eq(ubitint24_to_d128(-3942), ((unsigned long) -3942) & ((1 << 24) - 1)));
    assert(decimal128_eq(ubitint24_to_d128((1 << 23) - 1), (1 << 23) - 1));
    assert(decimal128_eq(ubitint24_to_d128((1 << 23)), (1 << 23)));
    assert(decimal128_eq(ubitint180_to_d128((struct S3){{0, 1, 0}}), 1 + (double) ~0ull));
    assert(decimal128_eq(ubitint180_to_d128((struct S3){{0, 0, 1}}), 1 + ~0ull * (double) ~0ull));

    assert(MASK(d32_to_bitint24(3.14159e3), 24) == 3141);
    assert(MASK(d32_to_bitint24(-3.14159e3), 24) == MASK(-3141, 24));
    assert(MASK(d32_to_ubitint24(3.14159e3), 24) == 3141);
    assert(MASK(d32_to_ubitint24((1 << 23)), 24) == (1 << 23));
    assert(MASK(d32_to_ubitint24((1 << 23) - 1), 24) == (1 << 23) - 1);

    assert(MASK(d64_to_bitint24(3.14159e3), 24) == 3141);
    assert(MASK(d64_to_bitint24(-3.14159e3), 24) == MASK(-3141, 24));
    assert(MASK(d64_to_ubitint24(3.14159e3), 24) == 3141);
    assert(MASK(d64_to_ubitint24((1 << 23)), 24) == (1 << 23));
    assert(MASK(d64_to_ubitint24((1 << 23) - 1), 24) == (1 << 23) - 1);

    assert(MASK(d128_to_bitint24(3.14159e3), 24) == 3141);
    assert(MASK(d128_to_bitint24(-3.14159e3), 24) == MASK(-3141, 24));
    assert(MASK(d128_to_ubitint24(3.14159e3), 24) == 3141);
    assert(MASK(d128_to_ubitint24((1 << 23)), 24) == (1 << 23));
    assert(MASK(d128_to_ubitint24((1 << 23) - 1), 24) == (1 << 23) - 1);

    struct S3 s3 = d32_to_bitint180(~0ull);
    assert(s3.arr[0] == 18446740000000000000ull);
    assert(s3.arr[1] == 0);
    assert(MASK(s3.arr[2], 52) == 0);
    s3 = d32_to_bitint180(-(double) ~0ull);
    assert(s3.arr[0] == 1 +  ~18446740000000000000ull);
    assert(s3.arr[1] == ~0ull);
    assert(MASK(s3.arr[2], 52) == MASK(~0ull, 52));
    s3 = d32_to_bitint180(10 * (double) ~0ull);
    assert(s3.arr[0] == 18446703336614035456ull);
    assert(s3.arr[1] == 9);
    assert(MASK(s3.arr[2], 52) == 0);
    s3 = d32_to_ubitint180(~0ull);
    assert(s3.arr[0] == 18446740000000000000ull);
    assert(s3.arr[1] == 0);
    assert(MASK(s3.arr[2], 52) == 0);
    s3 = d32_to_ubitint180(10 * (double) ~0ull);
    assert(s3.arr[0] == 18446703336614035456ull);
    assert(s3.arr[1] == 9);
    assert(MASK(s3.arr[2], 52) == 0);

    s3 = d64_to_bitint180(~0ull);
    assert(s3.arr[0] == 18446744073709550000ull);
    assert(s3.arr[1] == 0);
    assert(MASK(s3.arr[2], 52) == 0);
    s3 = d64_to_bitint180(-(double) ~0ull);
    assert(s3.arr[0] == 1 +  ~18446744073709550000ull);
    assert(s3.arr[1] == ~0ull);
    assert(MASK(s3.arr[2], 52) == MASK(~0ull, 52));
    s3 = d64_to_bitint180(10 * (double) ~0ull);
    assert(s3.arr[0] == 18446744073709535456ull);
    assert(s3.arr[1] == 9);
    assert(MASK(s3.arr[2], 52) == 0);
    s3 = d64_to_bitint180(~0ull);
    assert(s3.arr[0] == 18446744073709550000ull);
    assert(s3.arr[1] == 0);
    assert(MASK(s3.arr[2], 52) == 0);
    s3 = d64_to_bitint180(10 * (double) ~0ull);
    assert(s3.arr[0] == 18446744073709535456ull);
    assert(s3.arr[1] == 9);
    assert(MASK(s3.arr[2], 52) == 0);

    s3 = d128_to_bitint180(~0ull);
    assert(s3.arr[0] == 18446744073709551615ull);
    assert(s3.arr[1] == 0);
    assert(MASK(s3.arr[2], 52) == 0);
    s3 = d128_to_bitint180(-(double) ~0ull);
    assert(s3.arr[0] == 0);
    assert(s3.arr[1] == ~0ull);
    assert(MASK(s3.arr[2], 52) == MASK(~0ull, 52));
    s3 = d128_to_bitint180(10 * (double) ~0ull);
    assert(s3.arr[0] == 0);
    assert(s3.arr[1] == 10);
    assert(MASK(s3.arr[2], 52) == 0);
    s3 = d128_to_bitint180(~0ull);
    assert(s3.arr[0] == 18446744073709551615ull);
    assert(s3.arr[1] == 0);
    assert(MASK(s3.arr[2], 52) == 0);
    s3 = d128_to_bitint180(10 * (double) ~0ull);
    assert(s3.arr[0] == 0);
    assert(s3.arr[1] == 10);
    assert(MASK(s3.arr[2], 52) == 0);
#endif
    return EXIT_SUCCESS;
}

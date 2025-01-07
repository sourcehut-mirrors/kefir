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

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

int main(void) {
    _Atomic char i8;
    _Atomic short i16;
    _Atomic int i32;
    _Atomic long i64;
    _Atomic float f32;
    _Atomic double f64;
    _Atomic long double ld;
    _Atomic(int *) ptr;

    int arr[256];

    for (long i = -4096; i < 4096; i++) {
#define TEST(_op, _sz, _type, _oper)                                                  \
    _sz = (_type) i;                                                                  \
    assert(_op##_##_sz(&_sz, (_type) ~i) == (_type) (((_type) i) _oper((_type) ~i))); \
    assert(_sz == (_type) (((_type) i) _oper((_type) ~i)))
#define TEST_INT(_op, _oper)      \
    TEST(_op, i8, char, _oper);   \
    TEST(_op, i16, short, _oper); \
    TEST(_op, i32, int, _oper);   \
    TEST(_op, i64, long, _oper)
#define TEST_FP(_op, _sz, _type, _oper, _eps)                                                               \
    _sz = (_type) i;                                                                                        \
    assert(fabs(_op##_##_sz(&_sz, (_type) ~i) - (_type) (((_type) i) _oper((_type) ~i))) < EPSILON_##_eps); \
    assert(fabs(_sz - (_type) (((_type) i) _oper((_type) ~i))) < EPSILON_##_eps)
#define TEST_LD(_op, _oper)                                                                                     \
    ld = (long double) i;                                                                                       \
    assert(fabsl(_op##_ld(&ld, (long double) ~i) - (long double) (((long double) i) _oper((long double) ~i))) < \
           EPSILON_LD);                                                                                         \
    assert(fabsl(ld - (long double) (((long double) i) _oper((long double) ~i))) < EPSILON_LD)
#define TEST_ALL_SIZES(_op, _oper)       \
    TEST_INT(_op, _oper);                \
    TEST_FP(_op, f32, float, _oper, F);  \
    TEST_FP(_op, f64, double, _oper, D); \
    TEST_LD(_op, _oper)

        TEST_ALL_SIZES(multiply, *);
        if (((char) ~i) != 0) {
            TEST_ALL_SIZES(divide, /);
            TEST_INT(modulo, %);
        }
        TEST_ALL_SIZES(add, +);
        TEST_ALL_SIZES(subtract, -);
        if ((~i) >= 0 && (~i) < 8) {
            if ((char) i >= 0) {
                TEST_INT(shl, <<);
            }
            TEST_INT(shr, >>);
        }
        TEST_INT(iand, &);
        TEST_INT(ior, |);
        TEST_INT(ixor, ^);

        ptr = (int *) arr;
        assert(add_ptr(&ptr, (unsigned char) i) == ((int *) arr) + ((unsigned char) i));
        assert(ptr == ((int *) arr) + ((unsigned char) i));

        ptr = (int *) &arr[256];
        assert(subtract_ptr(&ptr, (unsigned char) i) == ((int *) &arr[256]) - ((unsigned char) i));
        assert(ptr == ((int *) &arr[256]) - ((unsigned char) i));
    }
    return EXIT_SUCCESS;
}

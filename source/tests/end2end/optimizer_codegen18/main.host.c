/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
#include <string.h>
#include "./definitions.h"

#define EPSILON_D 1e-6
#define EPSILON_F 1e-3

static long sum5_proxy(int n, ...) {
    va_list args;
    va_start(args, n);
    long result = sum5(n, args);
    va_end(args);
    return result;
}

extern long sum6(int n, va_list args) {
    long result = 0;
    while (n--) {
        result += va_arg(args, long);
    }
    return result;
}

int main(void) {
    assert(sum(0) == 0l);
    assert(sum(0, 1l) == 0l);
    assert(sum(0, 1l, 2l, 3l) == 0l);
    assert(sum(1, 1l, 2l, 3l) == 1l);
    assert(sum(2, 1l, 2l, 3l) == 3l);
    assert(sum(3, 1l, 2l, 3l) == 6l);
    assert(sum(10, 1l, 2l, 3l, 4l, 5l, 6l, 7l, 8l, 9l, 10l) == 55l);

    assert(fabs(sumd(0)) < EPSILON_D);
    assert(fabs(sumd(0, 1.0)) < EPSILON_D);
    assert(fabs(sumd(0, 1.0, 2.0, 3.0)) < EPSILON_D);
    assert(fabs(sumd(1, 1.0, 2.0, 3.0) - 1.0) < EPSILON_D);
    assert(fabs(sumd(2, 1.0, 2.0, 3.0) - 3.0) < EPSILON_D);
    assert(fabs(sumd(3, 1.0, 2.0, 3.0) - 6.0) < EPSILON_D);
    assert(fabs(sumd(10, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0) - 55.0) < EPSILON_D);

    assert(sum1(0) == 0);
    assert(sum1(0, (struct Struct1){{0}}) == 0);
    assert(sum1(0, (struct Struct1){{0}}, (struct Struct1){{0}}) == 0);
    assert(sum1(1, (struct Struct1){{0}}, (struct Struct1){{0}}) == 0);
    assert(sum1(1, (struct Struct1){{1, 2, 3, 4, 5}}, (struct Struct1){{0}}) == 15);
    assert(sum1(1, (struct Struct1){{1, 2, 3, 4, 5}}, (struct Struct1){{6, 7, 8, 9, 10}}) == 15);
    assert(sum1(2, (struct Struct1){{1, 2, 3, 4, 5}}, (struct Struct1){{6, 7, 8, 9, 10}}) == 55);
    assert(sum1(3, (struct Struct1){{1, 2, 3, 4, 5}}, (struct Struct1){{6, 7, 8, 9, 10}},
                (struct Struct1){{100, 0, 1}}) == 156);

    struct Struct2 s2;
    s2 = sum2(0);
    assert(s2.a == 0 && fabs(s2.b) < EPSILON_D);
    s2 = sum2(0, (struct Struct2){1, 1.5});
    assert(s2.a == 0 && fabs(s2.b) < EPSILON_D);
    s2 = sum2(1, (struct Struct2){1, 1.5});
    assert(s2.a == 1 && fabs(s2.b - 1.5) < EPSILON_D);
    s2 = sum2(1, (struct Struct2){1, 1.5}, (struct Struct2){10, -7.6});
    assert(s2.a == 1 && fabs(s2.b - 1.5) < EPSILON_D);
    s2 = sum2(2, (struct Struct2){1, 1.5}, (struct Struct2){10, -7.6});
    assert(s2.a == 11 && fabs(s2.b + 6.1) < EPSILON_D);
    s2 = sum2(3, (struct Struct2){1, 1.5}, (struct Struct2){10, -7.6}, (struct Struct2){-3, 100.05});
    assert(s2.a == 8 && fabs(s2.b - 93.95) < EPSILON_D);
    s2 = sum2(4, (struct Struct2){1, 1.5}, (struct Struct2){10, -7.6}, (struct Struct2){0, 0.0},
              (struct Struct2){-3, 100.05});
    assert(s2.a == 8 && fabs(s2.b - 93.95) < EPSILON_D);
    s2 = sum2(5, (struct Struct2){1, 1.5}, (struct Struct2){10, -7.6}, (struct Struct2){0, 0.0},
              (struct Struct2){-3, 100.05}, (struct Struct2){67, -3.14});
    assert(s2.a == 75 && fabs(s2.b - 90.81) < EPSILON_D);
    s2 = sum2(6, (struct Struct2){1, 1.5}, (struct Struct2){10, -7.6}, (struct Struct2){0, 0.0},
              (struct Struct2){-3, 100.05}, (struct Struct2){67, -3.14}, (struct Struct2){9, 5.67});
    assert(s2.a == 84 && fabs(s2.b - 96.48) < EPSILON_D);
    s2 = sum2(7, (struct Struct2){1, 1.5}, (struct Struct2){10, -7.6}, (struct Struct2){0, 0.0},
              (struct Struct2){-3, 100.05}, (struct Struct2){67, -3.14}, (struct Struct2){9, 5.67},
              (struct Struct2){-100, -100.0});
    assert(s2.a == -16 && fabs(s2.b + 3.52) < EPSILON_D);

    assert(sum3(0) == 0);
    assert(sum3(0, (struct Struct3){10, 3}) == 0);
    assert(sum3(1, (struct Struct3){10, 3}) == 30);
    assert(sum3(2, (struct Struct3){10, 3}, (struct Struct3){-7, 2}) == 16);
    assert(sum3(3, (struct Struct3){10, 3}, (struct Struct3){-7, 2}, (struct Struct3){24, 10}) == 256);
    assert(sum3(4, (struct Struct3){10, 3}, (struct Struct3){-7, 2}, (struct Struct3){24, 10},
                (struct Struct3){-1, 100}) == 156);
    assert(sum3(5, (struct Struct3){10, 3}, (struct Struct3){-7, 2}, (struct Struct3){24, 10},
                (struct Struct3){-1, 100}, (struct Struct3){20, 6}) == 276);

    assert(sum4(0) - 0.0f < EPSILON_F);
    assert(sum4(0, (struct Struct4){1.5f, 2.0f}) - 0.0f < EPSILON_F);
    assert(sum4(1, (struct Struct4){1.5f, 2.0f}) - 3.0f < EPSILON_F);
    assert(sum4(2, (struct Struct4){1.5f, 2.0f}, (struct Struct4){6.5f, 5.3f}) - 37.45f < EPSILON_F);
    assert(sum4(3, (struct Struct4){1.5f, 2.0f}, (struct Struct4){0.0f, 0.5f}, (struct Struct4){6.5f, 5.3f}) - 37.45f <
           EPSILON_F);
    assert(sum4(4, (struct Struct4){1.5f, 2.0f}, (struct Struct4){0.0f, 0.5f}, (struct Struct4){6.5f, 5.3f},
                (struct Struct4){100.0f, 1.0f}) -
               137.45f <
           EPSILON_F);
    assert(sum4(5, (struct Struct4){1.5f, 2.0f}, (struct Struct4){0.0f, 0.5f}, (struct Struct4){6.5f, 5.3f},
                (struct Struct4){100.0f, 1.0f}, (struct Struct4){2.0f, 0.5f}) -
               138.45f <
           EPSILON_F);
    assert(sum4(6, (struct Struct4){1.5f, 2.0f}, (struct Struct4){0.0f, 0.5f}, (struct Struct4){6.5f, 5.3f},
                (struct Struct4){100.0f, 1.0f}, (struct Struct4){0.0f, 0.0f}, (struct Struct4){2.0f, 0.5f}) -
               138.45f <
           EPSILON_F);
    assert(sum4(7, (struct Struct4){1.5f, 2.0f}, (struct Struct4){0.0f, 0.5f}, (struct Struct4){6.5f, 5.3f},
                (struct Struct4){100.0f, 1.0f}, (struct Struct4){0.0f, 0.0f}, (struct Struct4){2.0f, 0.5f},
                (struct Struct4){-100.0f, 3.5f}) +
               211.55f <
           EPSILON_F);
    assert(sum4(8, (struct Struct4){1.5f, 2.0f}, (struct Struct4){0.0f, 0.5f}, (struct Struct4){6.5f, 5.3f},
                (struct Struct4){100.0f, 1.0f}, (struct Struct4){0.0f, 0.0f}, (struct Struct4){2.0f, 0.5f},
                (struct Struct4){-100.0f, 3.5f}, (struct Struct4){6.0f, 0.1f}) +
               210.95f <
           EPSILON_F);
    assert(sum4(9, (struct Struct4){1.5f, 2.0f}, (struct Struct4){0.0f, 0.5f}, (struct Struct4){6.5f, 5.3f},
                (struct Struct4){100.0f, 1.0f}, (struct Struct4){0.0f, 0.0f}, (struct Struct4){2.0f, 0.5f},
                (struct Struct4){-100.0f, 3.5f}, (struct Struct4){6.0f, 0.1f}, (struct Struct4){-50.0f, -2.0f}) +
               110.95f <
           EPSILON_F);

    assert(sum5_proxy(0) == 0);
    assert(sum5_proxy(0, 1l) == 0);
    assert(sum5_proxy(0, 1l, 2l) == 0);
    assert(sum5_proxy(1, 1l, 2l) == 1);
    assert(sum5_proxy(2, 1l, 2l) == 3);
    assert(sum5_proxy(10, 1l, 2l, 3l, 4l, 5l, 6l, 7l, 8l, 9l, 10l) == 55l);

    assert(sum6_proxy(0) == 0);
    assert(sum6_proxy(0, 1l) == 0);
    assert(sum6_proxy(0, 1l, 2l) == 0);
    assert(sum6_proxy(1, 1l, 2l) == 1);
    assert(sum6_proxy(2, 1l, 2l) == 3);
    assert(sum6_proxy(10, 1l, 2l, 3l, 4l, 5l, 6l, 7l, 8l, 9l, 10l) == 55l);

    return EXIT_SUCCESS;
}

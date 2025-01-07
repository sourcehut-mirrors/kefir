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
#include <string.h>
#include "./definitions.h"

long mul(long x, long y) {
    return x * y;
}

long mulp(struct Pair p) {
    return p.a * p.b;
}

long mulh(struct HugePair p) {
    return p.a * p.b;
}

static int dummy_fun_res = 0;

int dummy_fun(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int m, int n, int o, int p,
              int q, int r) {
    int res = a == b - 1 && b == c - 1 && c == d - 1 && d == e - 1 && e == f - 1 && f == g - 1 && g == h - 1 &&
              h == i - 1 && i == j - 1 && j == k - 1 && k == m - 1 && m == n - 1 && n == o - 1 && o == p - 1 &&
              p == q - 1 && q == r - 1;
    if (!res) {
        abort();
    }
    return dummy_fun_res;
}

int dummy_fun2(struct IPair a, long b, struct Pair c, long d) {
    int res = a.a == a.b - 1 && a.b == b - 1 && b == c.a - 1 && c.a == c.b - 1 && c.b == d - 1;
    if (!res) {
        abort();
    }
    return dummy_fun_res;
}

int dummy_fun3(struct IPair a, short b, short c, struct HugePair d, long e, struct IPair f) {
    int res = a.a == a.b - 1 && a.b == b - 1 && b == c - 1 && c == d.a - 1 && d.a == d.b - 1 && d.b == e - 1 &&
              e == f.a - 1 && f.a == f.b - 1;
    if (!res) {
        abort();
    }
    return dummy_fun_res;
}

int main(void) {
    for (int i = -10; i <= 10; i++) {
        dummy_fun_res = i;
        assert(dummy_test() == dummy_fun_res);
        assert(dummy_test2() == dummy_fun_res);
        assert(dummy_test3() == dummy_fun_res);
    }
    for (long x = -1000; x <= 1000; x++) {
        for (long y = -100; y <= 100; y++) {
            assert(test_hypot(x, y) == (x * x) + (y * y));
            assert(test_hypotp((struct Pair){x, y}) == (x * x) + (y * y));
            assert(test_hypoth((struct HugePair){.a = x, .b = y}) == (x * x) + (y * y));
        }
    }
    return EXIT_SUCCESS;
}

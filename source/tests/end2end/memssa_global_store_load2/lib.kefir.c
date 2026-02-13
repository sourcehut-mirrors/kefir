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

#include "./definitions.h"

_Thread_local char c1, c2, c3;
_Thread_local short s1, s2, s3;
_Thread_local int i1, i2, i3;
_Thread_local long l1, l2, l3;
_Thread_local float f1, f2, f3;
_Thread_local double d1, d2, d3;
_Thread_local long double ld1, ld2, ld3;
_Thread_local _Complex float cf1, cf2, cf3;
_Thread_local _Complex double cd1, cd2, cd3;
_Thread_local _Complex long double cld1, cld2, cld3;

char test_char(char x) {
    c1 = x;
    c2 = x + 1;
    c3 = x * 2;
    return c1 + c2 + c3;
}

short test_short(short x) {
    s1 = x;
    s2 = x + 1;
    s3 = x * 2;
    return s1 + s2 + s3;
}

int test_int(int x) {
    i1 = x;
    i2 = x + 1;
    i3 = x * 2;
    return i1 + i2 + i3;
}

long test_long(long x) {
    l1 = x;
    l2 = x + 1;
    l3 = x * 2;
    return l1 + l2 + l3;
}

float test_float(float x) {
    f1 = x;
    f2 = x + 1;
    f3 = x * 2;
    return f1 + f2 + f3;
}

double test_double(double x) {
    d1 = x;
    d2 = x + 1;
    d3 = x * 2;
    return d1 + d2 + d3;
}

long double test_ldouble(long double x) {
    ld1 = x;
    ld2 = x + 1;
    ld3 = x * 2;
    return ld1 + ld2 + ld3;
}

_Complex float test_cfloat(_Complex float x) {
    cf1 = x;
    cf2 = x + 1;
    cf3 = x * 2;
    return cf1 + cf2 + cf3;
}

_Complex double test_cdouble(_Complex double x) {
    cd1 = x;
    cd2 = x + 1;
    cd3 = x * 2;
    return cd1 + cd2 + cd3;
}

_Complex long double test_cldouble(_Complex long double x) {
    cld1 = x;
    cld2 = x + 1;
    cld3 = x * 2;
    return cld1 + cld2 + cld3;
}

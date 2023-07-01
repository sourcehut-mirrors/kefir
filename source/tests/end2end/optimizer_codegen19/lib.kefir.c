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

#include "./definitions.h"

long double get_pi(void) {
    return PI_LD;
}

long double get_e(void) {
    return E_LD;
}

long double addld(long double x, long double y) {
    return x + y;
}

long double subld(long double x, long double y) {
    return x - y;
}

long double mulld(long double x, long double y) {
    return x * y;
}

long double divld(long double x, long double y) {
    return x / y;
}

long double negld(long double x) {
    return -x;
}

int ldequals(long double x, long double y) {
    return x == y;
}

int ldgreater(long double x, long double y) {
    return x > y;
}

int ldlesser(long double x, long double y) {
    return x < y;
}

long double long_to_long_double(long x) {
    return (long double) x;
}

long double ulong_to_long_double(unsigned long x) {
    return (long double) x;
}

long double float_to_long_double(float x) {
    return (long double) x;
}

long double double_to_long_double(double x) {
    return (long double) x;
}

long long_double_to_long(long double x) {
    return (long) x;
}

unsigned long long_double_to_ulong(long double x) {
    return (unsigned long) x;
}

int long_double_trunc(long double x) {
    return x ? 1 : 0;
}

_Bool long_double_to_bool(long double x) {
    return (_Bool) x;
}

float long_double_to_float(long double x) {
    return (float) x;
}

double long_double_to_double(long double x) {
    return (double) x;
}

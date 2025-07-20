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

#include "./definitions.h"

constexpr long a = 123456789;
constexpr float b = 3.14159f;
constexpr double c = 3.1415926;
constexpr long double d = -2.718281828L;
constexpr _Complex float e = 1.234f + 187.0if;
constexpr _Complex double f = 3.234 - 187.0i;
constexpr _Complex long double g = 2.234L - 187.0iL;
constexpr void *h = (void *) 0;
constexpr const char *i = "Hello, world" + 1;
constexpr const __CHAR32_TYPE__ *j = L"Hello, world" + 2;

long get_a(void) {
    return a;
}

float get_b(void) {
    return b;
}

double get_c(void) {
    return c;
}

long double get_d(void) {
    return d;
}

_Complex float get_e(void) {
    return e;
}

_Complex double get_f(void) {
    return f;
}

_Complex long double get_g(void) {
    return g;
}

void *get_h(void) {
    return h;
}

const char *get_i(void) {
    return i;
}

const unsigned int *get_j(void) {
    return j;
}

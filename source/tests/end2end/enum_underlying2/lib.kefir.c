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

enum A : char { A1 };

enum B : signed char { B1 };

enum C : unsigned char { C1 };

enum D : signed short { D1 };

enum E : unsigned short { E1 };

enum F : signed int { F1 };

enum G : unsigned int { G1 };

enum H : signed long { H1 };

enum I : unsigned long { I1 };

enum J : signed long long { J1 };

enum K : unsigned long long { K1 };

enum M : const volatile short { M1 };

enum N : _Atomic char { N1 };

#define CHECK(_x)               \
    _Generic((_x),              \
        char: 1,                \
        signed char: 2,         \
        unsigned char: 3,       \
        signed short: 4,        \
        unsigned short: 5,      \
        signed int: 6,          \
        unsigned int: 7,        \
        long: 8,                \
        unsigned long: 9,       \
        long long: 10,          \
        unsigned long long: 11, \
        default: 0)

int arr[] = {
#define TEST(_x) CHECK(_x##1), CHECK((enum _x) _x##1), sizeof(_x##1), sizeof(enum _x)
    TEST(A), TEST(B), TEST(C), TEST(D), TEST(E), TEST(F), TEST(G),
    TEST(H), TEST(I), TEST(J), TEST(K), TEST(M), TEST(N)};

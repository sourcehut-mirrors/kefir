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

#define ___char char
#define __char ___char
#define _char __char
#define char _char

#define f(a) a *g
#define g(a) f(a)

#define TEST1(x) 1 + TEST3(x)
#define TEST2(x) 10 - TEST1(x)
#define TEST3(x) 2 * TEST2(x)

#define CAT2(a, b) a##b
#define CAT(a, b) CAT2(a, b)
#define AB(o) CAT(o, y)

#define TEST_X(x) ((void *) x)
#define VAL TEST_X(VAL)
#define TEST_Y(x) TEST_Y(x)

#define XYZ(a, b) a##b(a, b) + 1

#define CAST(x, y) ((x) y)

#define _STR(s) #s
#define STR(s) _STR(s)

const char *STR1 = STR(char);
const char *STR2 = STR(__char);
const char *STR3 = STR(f(2)(9));
const char *STR4 = STR(TEST1(100));
const char *STR5 = STR(CAT(A, B)(x));
const char *STR6 = STR(VAL);
const char *STR7 = STR(TEST_Y(VAL));
const char *STR8 = STR(TEST_Y(TEST_Y(1)));
const char *STR9 = STR(XYZ(X, YZ));
const char *STR10 = STR(CAST(int, CAST(float, a)));
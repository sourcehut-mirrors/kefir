/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#define _STR(s) #s
#define STR(s) _STR(s)

const char *STR1 = STR(char);
const char *STR2 = STR(__char);
const char *STR3 = STR(f(2)(9));
const char *STR4 = STR(TEST1(100));
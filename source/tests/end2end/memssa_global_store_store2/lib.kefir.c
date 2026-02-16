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

int x = 0;
volatile int y = 0;
_Atomic int z = 0;

int test1(int a) {
    x = 0;
    for (int i = 0; i < a; i++)
        x = 1;
    x = a;
    return x;
}

int test2(int a) {
    x = 0;
    for (int i = 0; i < a; i++)
        x++;
    x = a;
    return x;
}

int test3(int a) {
    y = 0;
    for (int i = 0; i < a; i++)
        y = 1;
    y = a;
    return y;
}

int test4(int a) {
    y = 0;
    for (int i = 0; i < a; i++)
        y++;
    y = a;
    return y;
}

int test5(int a) {
    z = 0;
    for (int i = 0; i < a; i++)
        z = 1;
    z = a;
    return z;
}

int test6(int a) {
    z = 0;
    for (int i = 0; i < a; i++)
        z++;
    z = a;
    return z;
}

int test7(int a) {
    x = 0;
    for (int i = 0; i < a; i++)
        fn(x++);
    x = a;
    return x;
}

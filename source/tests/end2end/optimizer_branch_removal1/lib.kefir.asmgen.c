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

extern void somefn(void);
extern void somefn2(void);

void test1() {
    if (0)
        somefn();
}

void test2() {
    if (0)
        somefn();
    else
        somefn2();
}

void test3() {
    if (1)
        somefn();
    else
        somefn2();
}

void test4() {
    do {
        somefn();
    } while (0);
}

void test5() {
    do {
        somefn();
    } while (1);
    somefn2();
}

int test6() {
    return 1 || *((volatile int *) 0);
}

int test7() {
    return 0 || (0 && *((volatile int *) 0));
}
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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./definitions.h"

volatile char a = 100;
volatile short b = 1000;
volatile int c = 10000;
volatile long d = 100000;

int main(void) {
    assert(getA() == 100);
    assert(getB() == 1000);
    assert(getC() == 10000);
    assert(getD() == 100000);
    setA(11);
    setB(22);
    setC(33);
    setD(44);
    assert(getA() == 11);
    assert(getB() == 22);
    assert(getC() == 33);
    assert(getD() == 44);

    for (int i = 1; i < 256; i++) {
        vla_test(1);
    }

    struct S s;
    setS(&s, 10);
    assert(sumS(&s) == 50);
    return EXIT_SUCCESS;
}

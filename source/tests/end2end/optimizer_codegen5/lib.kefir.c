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

long factorial(long x) {
    if (x < 0) {
        return -1;
    } else if (x <= 2) {
        return x;
    }

    long result = 1;
    while (x > 1) {
        result *= x--;
    }
    return result;
}

#define SEGFAULT (*(volatile _Bool *) 0 = *(volatile _Bool *) 1)

long factorial2(long x) {
    0 && SEGFAULT;
    1 || SEGFAULT;
    1 && (1 || SEGFAULT);
    0 || (0 && SEGFAULT);
    !((0 && SEGFAULT) || (1 || SEGFAULT)) && SEGFAULT;

    long result;
    if ((x > 0 || x == 0) && (x <= 3 && !(x == 3))) {
        result = x;
    } else if ((x <= 0 && x != 0) || x <= 2 /* dummy */) {
        result = -1;
    } else
        for (result = (x > 0 ? 1 : 0); x > 1 && /* dummies */ x != 0 && x > 0; result *= x, x--) {
            (x || SEGFAULT) && (!x && SEGFAULT) && SEGFAULT;
        }
    return result;
}

long dummy_factorial(long x) {
    void *array[8];
    array[0] = &&l1;
    array[1] = &&l2;
    array[2] = &&l3;
    array[3] = &&l4;
    array[4] = &&l5;
    array[5] = &&l6;
    array[6] = &&l7;
    array[7] = &&l8;

    if (x > 0 && x <= 20) {
        goto *array[x - 1];
    } else if (x < 0) {
        return -1;
    }
    return 0;

l1:
    return 1;
l2:
    return 2;
l3:
    return 6;
l4:
    return 24;
l5:
    return 120;
l6:
    return 720;
l7:
    return 5040;
l8:
    return 40320;
}
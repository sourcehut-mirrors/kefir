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

int test1(int c, int a, int b) {
    int r;
    if (c) {
        r = a + b;
    } else {
        r = a + b + 0;
    }
    return r;
}

int test2(int c, int a, int b) {
    int r;
    if (c) {
        r = a + b + 0;
    } else {
        r = 0 + a + b;
    }
    return r;
}

int test3(int c, int a, int b) {
    int r;
    if (c) {
        r = a - b;
    } else {
        r = a - b - 0;
    }
    return r;
}

int test4(int c, int a, int b) {
    int r;
    if (c) {
        r = a * b;
    } else {
        r = a * b * 1;
    }
    return r;
}

int test5(int c, int a, int b) {
    int r;
    if (c) {
        r = a * b * 1;
    } else {
        r = 1 * a * b;
    }
    return r;
}

int test6(int c, int a, int b) {
    int r;
    if (c) {
        r = a / b / 1;
    } else {
        r = a / 1 / b;
    }
    return r;
}

int test7(int c, int a, int b) {
    int r;
    if (c) {
        r = a << b;
    } else {
        r = a << b << 0;
    }
    return r;
}

int test8(int c, int a, int b) {
    int r;
    if (c) {
        r = a << b << 0;
    } else {
        r = a << 0 << b;
    }
    return r;
}

int test9(int c, int a, int b) {
    int r;
    if (c) {
        r = a >> b;
    } else {
        r = a >> b >> 0;
    }
    return r;
}

int test10(int c, int a, int b) {
    int r;
    if (c) {
        r = a >> b >> 0;
    } else {
        r = a >> 0 >> b;
    }
    return r;
}

unsigned int test11(int c, unsigned int a, unsigned int b) {
    int r;
    if (c) {
        r = a >> b;
    } else {
        r = a >> b >> 0;
    }
    return r;
}

unsigned int test12(int c, unsigned int a, unsigned int b) {
    int r;
    if (c) {
        r = a >> b >> 0;
    } else {
        r = a >> 0 >> b;
    }
    return r;
}
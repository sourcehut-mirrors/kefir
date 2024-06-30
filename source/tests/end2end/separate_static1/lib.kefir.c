/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

static int x = 123;
static _Thread_local int y = -0xcafe;

int *xptr = &x;

int *getx(void) {
    return &x;
}

int *getx1(void) {
    static int x = 10;
    return &x;
}

int *getx2(void) {
    static int x = 20;
    return &x;
}

int *getx3(void) {
    static _Thread_local x = -500;
    return &x;
}

int *getx4(void) {
    static int x = 5678;
    {
        static int x = 9876;
        return &x;
    }
}

int *gety(void) {
    return &y;
}

int *gety1(void) {
    static _Thread_local y = 0xbabe;
    return &y;
}

const char *function_name(void) {
    return __FUNCTION__;
}

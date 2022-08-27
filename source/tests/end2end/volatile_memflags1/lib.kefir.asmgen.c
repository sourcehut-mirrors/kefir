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

char getA() {
    return a;
}

short getB() {
    return b;
}

int getC() {
    return c;
}

long getD() {
    return d;
}

void setA(char x) {
    a = x;
}

void setB(short x) {
    b = x;
}

void setC(int x) {
    c = x;
}

void setD(long x) {
    d = x;
}

void setS(struct S *s, int val) {
    s->a = (char) val;
    s->b = (short) val;
    s->c = val;
    s->d = val;
    s->x = val;
}

long sumS(struct S *s) {
    return s->a + s->b + s->c + s->d + s->x;
}

void vla_test(int x) {
    {
        long arr[x];
        arr[0] = sizeof(arr) + arr[0];
    }
}
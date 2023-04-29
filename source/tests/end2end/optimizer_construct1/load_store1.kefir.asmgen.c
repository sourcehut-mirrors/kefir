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

extern unsigned char a;
extern unsigned short b;
extern unsigned int c;
extern unsigned long d;

extern signed char e;
extern signed short f;
extern signed int g;
extern signed long h;

void load_unsigned(void) {
    (void) a;
    (void) b;
    (void) c;
    (void) d;
}

void load_signed(void) {
    (void) e;
    (void) f;
    (void) g;
    (void) h;
}

void store(void) {
    a = 0;
    b = 1;
    c = 2;
    d = 3;

    e = 4;
    f = 5;
    g = 6;
    h = 7;
}

long locals(int x, char y, long z) {
    static _Thread_local short f = 1;
    return x + y * z / f;
}

void volatiles1() {
    static volatile char x;
    static volatile int y;
    static volatile long z;
    static volatile float *a;
    (void) a[x + y - z];
    x = 1;
    y = 2;
    z = 3;
    *a = 4.5;
}
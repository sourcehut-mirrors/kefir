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

extern int a;
extern _Thread_local int b;
int c;
_Thread_local int d;
static int e;
static _Thread_local int f;

[[deprecated]] extern int a;
[[deprecated]] extern _Thread_local int b;
[[deprecated]] int c;
[[deprecated]] _Thread_local int d;
[[deprecated]] static int e;
[[deprecated]] static _Thread_local int f;

int get1(void) {
    return a;
}

int get2(void) {
    return b;
}

int get3(void) {
    return c;
}

int get4(void) {
    return d;
}

int get5(void) {
    return e;
}

int get6(void) {
    return f;
}

int get8(void) {
    extern int h;
    [[deprecated]] extern int h;
    return h;
}

int get9(void) {
    extern _Thread_local int i;
    [[deprecated]] extern _Thread_local int i;
    return i;
}

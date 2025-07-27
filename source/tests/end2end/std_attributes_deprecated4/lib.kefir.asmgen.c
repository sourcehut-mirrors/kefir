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

[[deprecated]] extern int a;
[[deprecated]] extern _Thread_local int b;
[[deprecated]] int c;
[[deprecated]] _Thread_local int d;
[[deprecated]] static int e;
[[deprecated]] static _Thread_local int f;
[[deprecated]] constexpr int g = 0;

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

int get7(void) {
    return g;
}

int get8(void) {
    [[deprecated]] extern int h;
    return h;
}

int get9(void) {
    [[deprecated]] extern _Thread_local int i;
    return i;
}

int get10(void) {
    [[deprecated]] static int j;
    return j;
}

int get11(void) {
    [[deprecated]] static _Thread_local int l;
    return l;
}

int get12(void) {
    [[deprecated]] static constexpr int k = 0;
    return k;
}

int get13(void) {
    [[deprecated]] auto int m = 0;
    return m;
}

int get14(void) {
    [[deprecated]] register int n = 0;
    return n;
}

int get15(void) {
    [[deprecated]] constexpr int o = 0;
    return o;
}

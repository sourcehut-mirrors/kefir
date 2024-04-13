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

extern struct S {
    long a : 7, : 0;
    long b : 15, : 0;
    long c : 23, : 0;
    long d : 31, : 0;
    long e : 39, : 0;
    long f : 47, : 0;
    long g : 55, : 0;
    long h : 63, : 0;
} s;

long get_a(void) {
    return s.a;
}

long get_b(void) {
    return s.b;
}

long get_c(void) {
    return s.c;
}

long get_d(void) {
    return s.d;
}

long get_e(void) {
    return s.e;
}

long get_f(void) {
    return s.f;
}

long get_g(void) {
    return s.g;
}

long get_h(void) {
    return s.h;
}

void set_a(long x) {
    s.a = x;
}

void set_b(long x) {
    s.b = x;
}

void set_c(long x) {
    s.c = x;
}

void set_d(long x) {
    s.d = x;
}

void set_e(long x) {
    s.e = x;
}

void set_f(long x) {
    s.f = x;
}

void set_g(long x) {
    s.g = x;
}

void set_h(long x) {
    s.h = x;
}

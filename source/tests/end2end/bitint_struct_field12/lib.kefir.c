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

struct S1 {
    unsigned _BitInt(6) a : 5;
    unsigned _BitInt(14) b : 12;
    unsigned _BitInt(23) c : 20;
    unsigned _BitInt(31) d : 27;
    unsigned _BitInt(39) e : 35;
    unsigned _BitInt(47) f : 46;
    unsigned _BitInt(55) g : 53;
    unsigned _BitInt(60) h : 59;
    unsigned _BitInt(120) i : 100;
};

long get1(const struct S1 *s) {
    return s->a;
}

long get2(const struct S1 *s) {
    return s->b;
}

long get3(const struct S1 *s) {
    return s->c;
}

long get4(const struct S1 *s) {
    return s->d;
}

long get5(const struct S1 *s) {
    return s->e;
}

long get6(const struct S1 *s) {
    return s->f;
}

long get7(const struct S1 *s) {
    return s->g;
}

long get8(const struct S1 *s) {
    return s->h;
}

long get9(const struct S1 *s) {
    return s->i;
}

void set1(struct S1 *s, long x) {
    s->a = x;
}

void set2(struct S1 *s, long x) {
    s->b = x;
}

void set3(struct S1 *s, long x) {
    s->c = x;
}

void set4(struct S1 *s, long x) {
    s->d = x;
}

void set5(struct S1 *s, long x) {
    s->e = x;
}

void set6(struct S1 *s, long x) {
    s->f = x;
}

void set7(struct S1 *s, long x) {
    s->g = x;
}

void set8(struct S1 *s, long x) {
    s->h = x;
}

void set9(struct S1 *s, long x) {
    s->i = x;
}

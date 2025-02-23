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

inline long a() {}
inline float b() {}
inline double c() {}
inline long double d() {}
inline _Complex float e() {}
inline _Complex double f() {}
inline _Complex long double g() {}
inline struct S2 h() {}

void test1(struct S1 *s1) {
    s1->a = a();
    s1->b = b();
    s1->c = c();
    s1->d = d();
    s1->e = e();
    s1->f = f();
    s1->g = g();
    s1->h = h();
}

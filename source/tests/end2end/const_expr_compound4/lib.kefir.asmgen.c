/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
    int a;
    int b;
};

struct S2 {
    struct S1 a;
    struct S1 b;
    struct S1 c;
};

__constexpr struct S1 s1 = {1, 2, .a = 1000, 2000, .a = 20, 40};

__constexpr struct S1 s2 = {1, 2, .a = 1000, 2000, .a = 20, .b = 40};

__constexpr struct S1 s3 = {1, 2, .a = 1000, .b = 2000, .a = 20, 40};

__constexpr struct S2 s4 = {s1, s2, s3};

__constexpr struct S2 s5 = {(struct S1) {1, 2, .a = 1000, 2000, .a = 20, 40},
                            (const struct S1) {1, 2, .a = 1000, 2000, .a = 20, .b = 40},
                            (restrict volatile struct S1) {1, 2, .a = 1000, .b = 2000, .a = 20, 40}};

__constexpr struct S2 s6 = {(struct S1) {1, 2, .a = 1000, 2000, .a = 20, 40}, s2,
                            (struct S1) {1, 2, .a = 1000, .b = 2000, .a = 20, 40}};

_Static_assert(s1.a == 20);
_Static_assert(s1.b == 40);
_Static_assert(s2.a == 20);
_Static_assert(s2.b == 40);
_Static_assert(s3.a == 20);
_Static_assert(s3.b == 40);

_Static_assert(s4.a.a == 20);
_Static_assert(s4.a.b == 40);
_Static_assert(s4.b.a == 20);
_Static_assert(s4.b.b == 40);
_Static_assert(s4.c.a == 20);
_Static_assert(s4.c.b == 40);

_Static_assert(s5.a.a == 20);
_Static_assert(s5.a.b == 40);
_Static_assert(s5.b.a == 20);
_Static_assert(s5.b.b == 40);
_Static_assert(s5.c.a == 20);
_Static_assert(s5.c.b == 40);

_Static_assert(s6.a.a == 20);
_Static_assert(s6.a.b == 40);
_Static_assert(s6.b.a == 20);
_Static_assert(s6.b.b == 40);
_Static_assert(s6.c.a == 20);
_Static_assert(s6.c.b == 40);

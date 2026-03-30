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

#include "./definitions.h"

__constexpr struct S2 s2 = (S2_t) {2000, 5678, 45};

__constexpr const struct S1 s1 = (S1_t) {.a = 1000,
                                         .b = {123},
                                         .d = s2,
                                         {1, -20, 30},
                                         (const S2_t) {-123, 4521, 119},
                                         {{-1, -2, -3}, (S2_t) {110, 220, 330}, 1, 2, 3}};

__constexpr S3_t s3 = (volatile S3_t) {s1, (S2_t) {-200, 300, 10}};

_Static_assert(s1.a == 1000);
_Static_assert(s1.b.c == 123);
_Static_assert(s1.d.a == 2000);
_Static_assert(s1.d.b == 5678);
_Static_assert(s1.d.c == 45);
_Static_assert(s1.e.a == 1);
_Static_assert(s1.e.b == -20);
_Static_assert(s1.e.c == 30);
_Static_assert(s1.f.a == -123);
_Static_assert(s1.f.b == 4521);
_Static_assert(s1.f.c == 119);

_Static_assert(s3.s1.a == 1000);
_Static_assert(s3.s1.b.c == 123);
_Static_assert(s3.s1.d.a == 2000);
_Static_assert(s3.s1.d.b == 5678);
_Static_assert(s3.s1.d.c == 45);
_Static_assert(s3.s1.e.a == 1);
_Static_assert(s3.s1.e.b == -20);
_Static_assert(s3.s1.e.c == 30);
_Static_assert(s3.s1.f.a == -123);
_Static_assert(s3.s1.f.b == 4521);
_Static_assert(s3.s1.f.c == 119);
_Static_assert(s3.s2.a == -200);
_Static_assert(s3.s2.b == 300);
_Static_assert(s3.s2.c == 10);

void init(struct S3 *ptr) {
    ptr->s1.a = s3.s1.a;
    ptr->s1.b.c = s3.s1.b.c;
    ptr->s1.d.a = s3.s1.d.a;
    ptr->s1.d.b = s3.s1.d.b;
    ptr->s1.d.c = s3.s1.d.c;
    ptr->s1.e.a = s3.s1.e.a;
    ptr->s1.e.b = s3.s1.e.b;
    ptr->s1.e.c = s3.s1.e.c;
    ptr->s1.f.a = s3.s1.f.a;
    ptr->s1.f.b = s3.s1.f.b;
    ptr->s1.f.c = s3.s1.f.c;
    ptr->s2.a = s3.s2.a;
    ptr->s2.b = s3.s2.b;
    ptr->s2.c = s3.s2.c;
}

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
    char a;
    _Alignas(2) struct {
        long b;
        long c;
    } d;
} __attribute__((packed));

struct S1 s1 = {1, 2, 3};

_Static_assert(sizeof(struct S1) == 18);
_Static_assert(_Alignof(struct S1) == 2);
_Static_assert(__builtin_offsetof(struct S1, a) == 0);
_Static_assert(__builtin_offsetof(struct S1, d) == 2);
_Static_assert(__builtin_offsetof(struct S1, d.b) == 2);
_Static_assert(__builtin_offsetof(struct S1, d.c) == 10);
_Static_assert(sizeof(s1) == 18);
_Static_assert(_Alignof(s1) == 2);

struct S2 {
    char f;
    _Alignas(1) struct {
        long b;
        long c;
        char d;
    } a;
    _Alignas(16) char e;
} __attribute__((packed));

struct S2 s2 = {0, 1, 2, 3, 4};

_Static_assert(sizeof(struct S2) == 48);
_Static_assert(_Alignof(struct S2) == 16);
_Static_assert(__builtin_offsetof(struct S2, f) == 0);
_Static_assert(__builtin_offsetof(struct S2, a) == 1);
_Static_assert(__builtin_offsetof(struct S2, a.b) == 1);
_Static_assert(__builtin_offsetof(struct S2, a.c) == 9);
_Static_assert(__builtin_offsetof(struct S2, a.d) == 17);
_Static_assert(__builtin_offsetof(struct S2, e) == 32);
_Static_assert(sizeof(s2) == 48);
_Static_assert(_Alignof(s2) == 16);

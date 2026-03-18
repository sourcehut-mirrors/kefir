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

#pragma pack(1)
struct S1 {
    char a;
    long b;
};

struct S1 s1 = {1, 2};

_Static_assert(sizeof(struct S1) == sizeof(char) + sizeof(long));
_Static_assert(_Alignof(struct S1) == 1);
_Static_assert(__builtin_offsetof(struct S1, a) == 0);
_Static_assert(__builtin_offsetof(struct S1, b) == sizeof(char));
_Static_assert(sizeof(s1) == sizeof(char) + sizeof(long));
_Static_assert(_Alignof(s1) == 1);

#pragma pack(2)
struct S2 {
    char a;
    long b;
};

struct S2 s2 = {1, 2};

_Static_assert(sizeof(struct S2) == 10);
_Static_assert(_Alignof(struct S2) == 2);
_Static_assert(__builtin_offsetof(struct S2, a) == 0);
_Static_assert(__builtin_offsetof(struct S2, b) == 2);
_Static_assert(sizeof(s2) == 10);
_Static_assert(_Alignof(s2) == 2);

#pragma pack(4)
struct S3 {
    char a;
    long b;
};

struct S3 s3 = {1, 2};

_Static_assert(sizeof(struct S3) == 12);
_Static_assert(_Alignof(struct S3) == 4);
_Static_assert(__builtin_offsetof(struct S3, a) == 0);
_Static_assert(__builtin_offsetof(struct S3, b) == 4);
_Static_assert(sizeof(s3) == 12);
_Static_assert(_Alignof(s3) == 4);

#pragma pack(8)
struct S4 {
    char a;
    long b;
};

struct S4 s4 = {1, 2};

_Static_assert(sizeof(struct S4) == 16);
_Static_assert(_Alignof(struct S4) == 8);
_Static_assert(__builtin_offsetof(struct S4, a) == 0);
_Static_assert(__builtin_offsetof(struct S4, b) == 8);
_Static_assert(sizeof(s4) == 16);
_Static_assert(_Alignof(s4) == 8);

#pragma pack(16)
struct S5 {
    char a;
    long b;
};

struct S5 s5 = {1, 2};

_Static_assert(sizeof(struct S5) == 16);
_Static_assert(_Alignof(struct S5) == 8);
_Static_assert(__builtin_offsetof(struct S5, a) == 0);
_Static_assert(__builtin_offsetof(struct S5, b) == 8);
_Static_assert(sizeof(s5) == 16);
_Static_assert(_Alignof(s5) == 8);

#pragma pack(4)
struct S6 {
    char a;
    short b;
};

struct S6 s6 = {1, 2};

_Static_assert(sizeof(struct S6) == 4);
_Static_assert(_Alignof(struct S6) == 2);
_Static_assert(__builtin_offsetof(struct S6, a) == 0);
_Static_assert(__builtin_offsetof(struct S6, b) == 2);
_Static_assert(sizeof(s6) == 4);
_Static_assert(_Alignof(s6) == 2);

#pragma pack(4)
struct S7 {
    char a;
    _Alignas(16) short b;
};

struct S7 s7 = {1, 2};

_Static_assert(sizeof(struct S7) == 8);
_Static_assert(_Alignof(struct S7) == 4);
_Static_assert(__builtin_offsetof(struct S7, a) == 0);
_Static_assert(__builtin_offsetof(struct S7, b) == 4);
_Static_assert(sizeof(s7) == 8);
_Static_assert(_Alignof(s7) == 4);

#pragma pack(2)
struct S8 {
    char a;
    struct {
        long b;
        long c;
    } d;
};

struct S8 s8 = {1, 2};

_Static_assert(sizeof(struct S8) == 18);
_Static_assert(_Alignof(struct S8) == 2);
_Static_assert(__builtin_offsetof(struct S8, a) == 0);
_Static_assert(__builtin_offsetof(struct S8, d) == 2);
_Static_assert(__builtin_offsetof(struct S8, d.b) == 2);
_Static_assert(__builtin_offsetof(struct S8, d.c) == 10);
_Static_assert(sizeof(s8) == 18);
_Static_assert(_Alignof(s8) == 2);

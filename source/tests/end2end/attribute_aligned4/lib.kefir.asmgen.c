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

enum E1 { A1 } __attribute__((aligned(1)));

enum E1 e1 = A1;

_Static_assert(sizeof(enum E1) == sizeof(unsigned int));
_Static_assert(_Alignof(enum E1) == 1);
_Static_assert(sizeof(e1) == sizeof(unsigned int));
_Static_assert(_Alignof(e1) == 1);

enum E2 { A2 } __attribute__((aligned(64)));

enum E2 e2 = A2;

_Static_assert(sizeof(enum E2) == sizeof(unsigned int));
_Static_assert(_Alignof(enum E2) == 64);
_Static_assert(sizeof(e2) == sizeof(unsigned int));
_Static_assert(_Alignof(e2) == 64);

enum E3 : char { A3 } __attribute__((aligned(4)));

enum E3 e3 = A3;

_Static_assert(sizeof(enum E3) == sizeof(char));
_Static_assert(_Alignof(enum E3) == 4);
_Static_assert(sizeof(e3) == sizeof(char));
_Static_assert(_Alignof(e3) == 4);

enum E4 : unsigned long { A4 } __attribute__((aligned(2)));

enum E4 e4 = A4;

_Static_assert(sizeof(enum E4) == sizeof(long));
_Static_assert(_Alignof(enum E4) == 2);
_Static_assert(sizeof(e4) == sizeof(long));
_Static_assert(_Alignof(e4) == 2);

struct S1 {
    enum E1 a;
};

struct S1 s1 = {0};

_Static_assert(sizeof(struct S1) == sizeof(unsigned int));
_Static_assert(_Alignof(struct S1) == 1);
_Static_assert(sizeof(s1) == sizeof(unsigned int));
_Static_assert(_Alignof(s1) == 1);

struct S2 {
    enum E2 a;
};

struct S2 s2 = {0};

_Static_assert(sizeof(struct S2) == 64);
_Static_assert(_Alignof(struct S2) == 64);
_Static_assert(sizeof(s2) == 64);
_Static_assert(_Alignof(s2) == 64);

struct S3 {
    enum E3 a;
};

struct S3 s3 = {0};

_Static_assert(sizeof(struct S3) == 4);
_Static_assert(_Alignof(struct S3) == 4);
_Static_assert(sizeof(s3) == 4);
_Static_assert(_Alignof(s3) == 4);

struct S4 {
    enum E4 a;
};

struct S4 s4 = {0};

_Static_assert(sizeof(struct S4) == 8);
_Static_assert(_Alignof(struct S4) == 2);
_Static_assert(sizeof(s4) == 8);
_Static_assert(_Alignof(s4) == 2);

enum E4 __attribute__((aligned(1))) e5 = A4;

_Static_assert(sizeof(e5) == sizeof(long));
_Static_assert(_Alignof(e5) == 1);

struct S5 {
    __attribute__((aligned(1))) enum E4 a;
};

struct S5 s5 = {0};

_Static_assert(sizeof(struct S5) == 8);
_Static_assert(_Alignof(struct S5) == 2);
_Static_assert(sizeof(s5) == 8);
_Static_assert(_Alignof(s5) == 2);

enum E4 __attribute__((aligned(128))) e6 = A4;

_Static_assert(sizeof(e6) == sizeof(long));
_Static_assert(_Alignof(e6) == 128);

struct S6 {
    __attribute__((aligned(64))) enum E4 a;
};

struct S6 s6 = {0};

_Static_assert(sizeof(struct S6) == 64);
_Static_assert(_Alignof(struct S6) == 64);
_Static_assert(sizeof(s6) == 64);
_Static_assert(_Alignof(s6) == 64);

struct S7 {
    char x;
    __attribute__((aligned(64))) enum E4 a;
};

struct S7 s7 = {0};

_Static_assert(sizeof(struct S7) == 128);
_Static_assert(_Alignof(struct S7) == 64);
_Static_assert(__builtin_offsetof(struct S7, a) == 64);
_Static_assert(sizeof(s7) == 128);
_Static_assert(_Alignof(s7) == 64);

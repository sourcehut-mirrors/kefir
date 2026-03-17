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
    short b;
} __attribute__((aligned(8)));

struct S1 s1 = {1, 2};
struct S1 s2[2] = {{1, 2}, {3, 4}};
__attribute__((aligned(16))) struct S1 s3 = {1, 2};

_Static_assert(sizeof(struct S1) == 8);
_Static_assert(_Alignof(struct S1) == 8);
_Static_assert(sizeof(s1) == 8);
_Static_assert(_Alignof(s1) == 8);
_Static_assert(sizeof(s2) == 16);
_Static_assert(_Alignof(s2) == 8);
_Static_assert(sizeof(s3) == 8);
_Static_assert(_Alignof(s3) == 16);

struct S2 {
    char a;
    long b;
} __attribute__((aligned(1)));

struct S2 s4 = {0};
__attribute__((aligned(32))) struct S2 s5[2] = {{0}};

_Static_assert(sizeof(struct S2) == 2 * sizeof(long));
_Static_assert(_Alignof(struct S2) == _Alignof(long));
_Static_assert(sizeof(s4) == 2 * sizeof(long));
_Static_assert(_Alignof(s4) == _Alignof(long));
_Static_assert(sizeof(s5) == 4 * sizeof(long));
_Static_assert(_Alignof(s5) == 32);

struct S3 {
    char a;
    long b;
} __attribute__((aligned(1), packed));

struct S3 s6 = {1, 2};
__attribute__((aligned(4))) struct S3 s7 = {1, 2};
__attribute__((aligned(32))) struct S3 s8 = {1, 2};
__attribute__((aligned(4))) struct S3 s9[2] = {{1, 2}, {3, 4}};
struct S3 s10[2] = {{1, 2}, {3, 4}};

_Static_assert(sizeof(struct S3) == sizeof(char) + sizeof(long));
_Static_assert(_Alignof(struct S3) == 1);
_Static_assert(sizeof(s6) == sizeof(char) + sizeof(long));
_Static_assert(_Alignof(s6) == 1);
_Static_assert(sizeof(s7) == 9);
_Static_assert(_Alignof(s7) == 4);
_Static_assert(sizeof(s8) == 9);
_Static_assert(_Alignof(s8) == 32);
_Static_assert(sizeof(s9) == 18);
_Static_assert(_Alignof(s9) == 4);
_Static_assert(sizeof(s10) == 18);
_Static_assert(_Alignof(s10) == 1);

struct S4 {
    char a;
    long b;
} __attribute__((aligned(2), packed));

struct S4 s11 = {1, 2};
__attribute__((aligned(1))) struct S4 s12 = {1, 2};
__attribute__((aligned(16))) struct S4 s13 = {1, 2};
__attribute__((aligned(1))) struct S4 s14[2] = {{1, 2}, {3, 4}};
__attribute__((aligned(32))) struct S4 s15[2] = {{1, 2}, {3, 4}};

_Static_assert(sizeof(struct S4) == 1 + sizeof(char) + sizeof(long));
_Static_assert(_Alignof(struct S4) == 2);
_Static_assert(sizeof(s11) == 1 + sizeof(char) + sizeof(long));
_Static_assert(_Alignof(s11) == 2);
_Static_assert(sizeof(s12) == 1 + sizeof(char) + sizeof(long));
_Static_assert(_Alignof(s12) == 1);
_Static_assert(sizeof(s13) == 1 + sizeof(char) + sizeof(long));
_Static_assert(_Alignof(s13) == 16);
_Static_assert(sizeof(s14) == 2 * (1 + sizeof(char) + sizeof(long)));
_Static_assert(_Alignof(s14) == 1);
_Static_assert(sizeof(s15) == 2 * (1 + sizeof(char) + sizeof(long)));
_Static_assert(_Alignof(s15) == 32);

struct S5 {
    char a;
    struct S3 b;
};

struct S5 s16 = {1, {2, 3}};

_Static_assert(sizeof(struct S5) == sizeof(char) + sizeof(char) + sizeof(long));
_Static_assert(_Alignof(struct S5) == 1);
_Static_assert(__builtin_offsetof(struct S5, b) == 1);
_Static_assert(sizeof(s16) == sizeof(char) + sizeof(char) + sizeof(long));
_Static_assert(_Alignof(s16) == 1);

struct S6 {
    char a;
    struct S4 b;
};

struct S6 s17 = {1, {2, 3}};

_Static_assert(sizeof(struct S6) == 12);
_Static_assert(_Alignof(struct S6) == 2);
_Static_assert(__builtin_offsetof(struct S6, b) == 2);
_Static_assert(sizeof(s17) == 12);
_Static_assert(_Alignof(s17) == 2);

__attribute__((aligned(1))) struct S6 s18 = {1, {2, 3}};

_Static_assert(sizeof(s18) == 12);
_Static_assert(_Alignof(s18) == 1);

struct S7 {
    char a;
    __attribute__((aligned(1))) struct S4 b;
};

struct S7 s19 = {1, {2, 3}};

_Static_assert(sizeof(struct S7) == 12);
_Static_assert(_Alignof(struct S7) == 2);
_Static_assert(__builtin_offsetof(struct S7, b) == 2);
_Static_assert(sizeof(s19) == 12);
_Static_assert(_Alignof(s19) == 2);

__attribute__((aligned(32))) struct S6 s20 = {1, {2, 3}};

_Static_assert(sizeof(s20) == 12);
_Static_assert(_Alignof(s20) == 32);

struct S8 {
    char a;
    __attribute__((aligned(32))) struct S4 b;
};

struct S8 s21 = {1, {2, 3}};

_Static_assert(sizeof(struct S8) == 64);
_Static_assert(_Alignof(struct S8) == 32);
_Static_assert(__builtin_offsetof(struct S8, b) == 32);
_Static_assert(sizeof(s21) == 64);
_Static_assert(_Alignof(s21) == 32);
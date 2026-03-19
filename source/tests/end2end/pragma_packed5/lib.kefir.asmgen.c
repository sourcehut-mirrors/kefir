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

#define CONTENTS                   \
    char f0;                       \
    unsigned int f1;               \
    unsigned long f2;              \
    const volatile __int128 f3;    \
    volatile unsigned __int128 f4; \
    volatile unsigned int f5;      \
    volatile unsigned int f6;

#pragma pack(push)
#pragma pack(1)
struct S2 {
    CONTENTS;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(2)
struct S3 {
    CONTENTS;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(4)
struct S4 {
    CONTENTS;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(8)
struct S5 {
    CONTENTS;
};
#pragma pack(pop)

#pragma pack(push)
#pragma pack(16)
struct S6 {
    CONTENTS;
};
#pragma pack(pop)

_Static_assert(sizeof(struct S2) == 53);
_Static_assert(_Alignof(struct S2) == 1);
_Static_assert(__builtin_offsetof(struct S2, f0) == 0);
_Static_assert(__builtin_offsetof(struct S2, f1) == 1);
_Static_assert(__builtin_offsetof(struct S2, f2) == 5);
_Static_assert(__builtin_offsetof(struct S2, f3) == 13);
_Static_assert(__builtin_offsetof(struct S2, f4) == 29);
_Static_assert(__builtin_offsetof(struct S2, f5) == 45);
_Static_assert(__builtin_offsetof(struct S2, f6) == 49);

_Static_assert(sizeof(struct S3) == 54);
_Static_assert(_Alignof(struct S3) == 2);
_Static_assert(__builtin_offsetof(struct S3, f0) == 0);
_Static_assert(__builtin_offsetof(struct S3, f1) == 2);
_Static_assert(__builtin_offsetof(struct S3, f2) == 6);
_Static_assert(__builtin_offsetof(struct S3, f3) == 14);
_Static_assert(__builtin_offsetof(struct S3, f4) == 30);
_Static_assert(__builtin_offsetof(struct S3, f5) == 46);
_Static_assert(__builtin_offsetof(struct S3, f6) == 50);

_Static_assert(sizeof(struct S4) == 56);
_Static_assert(_Alignof(struct S4) == 4);
_Static_assert(__builtin_offsetof(struct S4, f0) == 0);
_Static_assert(__builtin_offsetof(struct S4, f1) == 4);
_Static_assert(__builtin_offsetof(struct S4, f2) == 8);
_Static_assert(__builtin_offsetof(struct S4, f3) == 16);
_Static_assert(__builtin_offsetof(struct S4, f4) == 32);
_Static_assert(__builtin_offsetof(struct S4, f5) == 48);
_Static_assert(__builtin_offsetof(struct S4, f6) == 52);

_Static_assert(sizeof(struct S5) == 56);
_Static_assert(_Alignof(struct S5) == 8);
_Static_assert(__builtin_offsetof(struct S5, f0) == 0);
_Static_assert(__builtin_offsetof(struct S5, f1) == 4);
_Static_assert(__builtin_offsetof(struct S5, f2) == 8);
_Static_assert(__builtin_offsetof(struct S5, f3) == 16);
_Static_assert(__builtin_offsetof(struct S5, f4) == 32);
_Static_assert(__builtin_offsetof(struct S5, f5) == 48);
_Static_assert(__builtin_offsetof(struct S5, f6) == 52);

_Static_assert(sizeof(struct S6) == 64);
_Static_assert(_Alignof(struct S6) == 16);
_Static_assert(__builtin_offsetof(struct S6, f0) == 0);
_Static_assert(__builtin_offsetof(struct S6, f1) == 4);
_Static_assert(__builtin_offsetof(struct S6, f2) == 8);
_Static_assert(__builtin_offsetof(struct S6, f3) == 16);
_Static_assert(__builtin_offsetof(struct S6, f4) == 32);
_Static_assert(__builtin_offsetof(struct S6, f5) == 48);
_Static_assert(__builtin_offsetof(struct S6, f6) == 52);

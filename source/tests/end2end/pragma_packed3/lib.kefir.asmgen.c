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
struct S0 {
    int f0;
    volatile unsigned short f1;
    int f2;
    unsigned char f3;
    volatile unsigned __int128 f4;
    unsigned char f5;
    short f6;
    unsigned short f7;
};

#pragma pack(push)
#pragma pack(1)
struct S1 {
    volatile unsigned long f0;
    struct S0 f1;
    long f2;
    volatile __int128 f3;
    volatile unsigned f4 : 24;
    const volatile char f5;
    long f6;
    char f7;
    int f8;
    struct S0 f9;
};
#pragma pack(pop)

_Static_assert(sizeof(struct S1) == 145);
_Static_assert(_Alignof(struct S1) == 1);
_Static_assert(__builtin_offsetof(struct S1, f0) == 0);
_Static_assert(__builtin_offsetof(struct S1, f1) == 8);
_Static_assert(__builtin_offsetof(struct S1, f2) == 56);
_Static_assert(__builtin_offsetof(struct S1, f3) == 64);
_Static_assert(__builtin_offsetof(struct S1, f4) == 80);
_Static_assert(__builtin_offsetof(struct S1, f5) == 83);
_Static_assert(__builtin_offsetof(struct S1, f6) == 84);
_Static_assert(__builtin_offsetof(struct S1, f7) == 92);
_Static_assert(__builtin_offsetof(struct S1, f8) == 93);
_Static_assert(__builtin_offsetof(struct S1, f9) == 97);

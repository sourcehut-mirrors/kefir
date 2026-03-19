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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#if defined(__GNUC__) || defined(__clang__) || defined(__KEFIRCC__)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#define CONTENTS               \
    volatile unsigned long f0; \
    struct S0 f1;              \
    long f2;                   \
    volatile __int128 f3;      \
    volatile unsigned f4 : 24; \
    const volatile char f5;    \
    long f6;                   \
    char f7;                   \
    int f8;                    \
    struct S0 f9

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

struct S2 {
    CONTENTS;
} __attribute__((packed));

extern struct S2 s2;

#pragma GCC diagnostic pop

#endif

#endif

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

#define CONTENTS                   \
    const unsigned f0 : 9;         \
    unsigned f1 : 26;              \
    signed f2 : 24;                \
    const volatile signed f3 : 11; \
    const signed f4 : 22;          \
    volatile struct S1 f5;         \
    volatile unsigned f6 : 4;      \
    unsigned f7 : 11;              \
    unsigned f8 : 30

struct S1 {
    volatile unsigned __int128 f0;
    unsigned f1 : 11;
    volatile signed f2 : 29;
    volatile unsigned f3 : 31;
    volatile long f4;
    signed : 0;
};

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

extern struct S2 s2;
extern struct S3 s3;
extern struct S4 s4;
extern struct S5 s5;
extern struct S6 s6;

#pragma GCC diagnostic pop

#endif

#endif

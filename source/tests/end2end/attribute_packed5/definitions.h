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

#define CONTENTS      \
    unsigned char f0; \
    int f1;           \
    struct S0 f2;     \
    unsigned int f3;  \
    unsigned long f4

struct S0 {
    unsigned int f0;
    unsigned __int128 f1;
    signed f2 : 4;
    unsigned f3 : 22;
};

struct S2 {
    CONTENTS;
} __attribute((packed));

extern struct S2 s2;

#pragma GCC diagnostic pop

#endif

#endif

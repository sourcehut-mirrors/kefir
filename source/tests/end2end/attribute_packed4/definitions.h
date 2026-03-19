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

#define CONTENTS              \
    signed f0 : 1;            \
    unsigned f1 : 26;         \
    signed f2 : 13;           \
    volatile unsigned f3 : 9; \
    unsigned : 0;             \
    unsigned f4 : 9;          \
    signed f5 : 15;           \
    volatile signed f6 : 30;  \
    signed f7 : 11

struct S2 {
    CONTENTS;
} __attribute__((packed));

extern struct S2 s2;

#pragma GCC diagnostic pop

#endif

#endif

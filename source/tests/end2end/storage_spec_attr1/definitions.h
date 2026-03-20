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

__attribute((aligned(16))) typedef struct S1 {
    char x;
} s1t;
typedef __attribute((aligned(16))) struct S2 {
    char x;
} s2t;
typedef struct __attribute((aligned(16))) S3 {
    char x;
} s3t;
typedef struct S4 {
    char x;
} __attribute((aligned(16))) s4t;
typedef struct S5 {
    char x;
} s5t __attribute((aligned(16)));

extern int arr[];

#pragma GCC diagnostic pop

#endif

#endif

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

typedef struct S2 {
    int a;
    long b;
    char c;
} S2_t;

typedef struct S1 {
    int a;
    struct {
        long c;
    } b;
    S2_t d;
    S2_t e;
    S2_t f;
} S1_t;

typedef struct S3 {
    struct S1 s1;
    S2_t s2;
} S3_t;

void init(struct S3 *);

#endif

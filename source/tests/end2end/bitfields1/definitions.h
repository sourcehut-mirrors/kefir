/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

struct Struct1 {
    long lng1;
    long lng2;
    char chr1;
    int int1 : 6;
    union {
        struct {
            short shr1;
            short shr2;
        };
        int int2;
    };
};

struct Struct2 {
    long i1 : 3, i2 : 22, i3 : 5, i4 : 11, i5 : 10;
    short i6 : 7, : 5;
    char : 2, i7 : 5;
};

#define STRUCT1_CALCULATE(s)                                                                    \
    ((long) (((s.lng1 + s.lng2 - s.chr1) * s.int1 + s.shr1 - s.shr2) * sizeof(struct Struct1) - \
             _Alignof(struct Struct1)))
long struct1_calculate(struct Struct1);

extern struct Struct1 STRUCTURE;
extern struct Struct2 STRUCTURE2;

void fill(struct Struct2 *);

#endif

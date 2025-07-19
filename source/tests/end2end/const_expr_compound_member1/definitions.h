/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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

struct S2 {
    long arr[5];
};

struct S1 {
    int a;
    struct {
        float c;
        struct S2 d;
    } b;
    double e;
    struct {
        long f;
    };
};

extern struct S1 s1;
extern long *a;
extern double b;
extern long c;
extern _Complex float d;
extern long **e;
extern void *f;
extern long g;
extern long double h;
extern _Complex double i;
extern const char *j;

#endif

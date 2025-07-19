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

struct S1 {
    int a;
    float b;
    const char c[10];
    long d;
    const char *e;
};

struct S2 {
    int a;
    struct {
        struct S1 b;
        float c;
    } d;
    struct S1 e;
};

union S3 {
    int a;
    struct S2 b;
};

struct S3_2 {
    char a[10];
};

struct S4 {
    struct S3_2 a;
    long b;
    union S3 c;
};

extern struct S1 s1_1;
extern struct S1 s1_2;
extern struct S2 s2;
extern union S3 s3_1;
extern struct S3_2 s3_2;

#endif

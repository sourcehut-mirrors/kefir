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
};

struct S2 {
    int a;
    long b;
};

struct S3 {
    int a;
    long b;
    double c;
};

struct S4 {
    int a;
    long b;
    double c;
    void *d;
};

struct S5 {
    int a;
    long b;
    double c;
    void *d;
    char x;
    _Alignas(_Alignof(long) * 2) short e;
};

int get0(void);
struct S1 get1(void);
struct S2 get2(void);
struct S3 get3(void);
struct S4 get4(void);
struct S5 get5(void);
_Complex long double getx(void);

#endif

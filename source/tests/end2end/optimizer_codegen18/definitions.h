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

#ifdef __KEFIRCC__
#define va_list __builtin_va_list
#else
#include <stdarg.h>
#endif

extern int sum(int, ...);
extern double sumd(int, ...);

struct Struct1 {
    long arr[5];
};

extern long sum1(int, ...);

struct Struct2 {
    int a;
    double b;
};

extern struct Struct2 sum2(int, ...);

struct Struct3 {
    long a;
    long b;
};

extern long sum3(int, ...);

struct Struct4 {
    float a;
    float b;
};

extern float sum4(int, ...);

extern long sum5(int, va_list);

extern long sum6(int, va_list);
extern long sum6_proxy(int, ...);

extern long sum7(int, ...);

#endif

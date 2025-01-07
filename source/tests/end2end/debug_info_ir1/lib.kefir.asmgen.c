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

_Bool a1;
char a2;
unsigned char a3;
signed char a4;
unsigned short a5;
signed short a6;
unsigned int a7;
signed int a8;
unsigned long a9;
signed long a10;
unsigned long long a11;
signed long long a12;
float a13;
double a14;
long double a16;
_Complex float a17;
_Complex double a18;
_Complex long double a19;

void *b1;
const char *b2;
extern int b3[];
int b4[100];

struct S1 {
    int a;
    long b : 5;
    short c : 2;
    union {
        struct {
            int e;
            long f;
        };
        struct {
            float g[3];
        };
    } d;
    _Alignas(long double) char h;
    struct S1 *i;
    int (*j)(int, long, short);
    int pad[];
} c1;

extern double d1(int, short, long double);
float d2(a, b, c)
    long a;
    struct S1 b;
    void **c; {
    int vla[a];
    return 0.0f;
}

__builtin_va_list e1;
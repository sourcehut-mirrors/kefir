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

#ifdef __x86_64__
double custom_hypot(double, double);

struct S1 {
    long l;
    int i;
    short s;
    char c;
};

struct S1 init_s1(void);

long clear8(long);
long clear16(long);
long clear32(long);
long clear64(long);

long set8(long);
long set16(long);
long set32(long);
long set64(long);

struct S2 {
    unsigned short shrt[2];
};

long sum3_one(long, long, long);
struct S2 make_s2(unsigned int);
unsigned int unwrap_s2(struct S2);
unsigned int cast_int(unsigned long);

extern void *array[];
extern void fn1(void);
extern int *getx(void);
extern void init_array(void);

#endif

#endif

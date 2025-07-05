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
    unsigned char a : 5;
    unsigned long b : 45;
    unsigned short c : 10;
    unsigned long d : 30;
    unsigned int e : 25;
    unsigned short f : 8;
    unsigned long g : 23;
    unsigned long h : 15;
    unsigned char i : 4;
};

void set1(struct S1 *, long);
void set2(struct S1 *, long);
void set3(struct S1 *, long);
void set4(struct S1 *, long);
void set5(struct S1 *, long);
void set6(struct S1 *, long);
void set7(struct S1 *, long);
void set8(struct S1 *, long);
void set9(struct S1 *, long);

long get1(const struct S1 *);
long get2(const struct S1 *);
long get3(const struct S1 *);
long get4(const struct S1 *);
long get5(const struct S1 *);
long get6(const struct S1 *);
long get7(const struct S1 *);
long get8(const struct S1 *);
long get9(const struct S1 *);

struct S2 {
    unsigned long arr[7];
};

void set10(struct S2 *, long);
void set11(struct S2 *, long);
void set12(struct S2 *, long);

long get10(const struct S2 *);
long get11(const struct S2 *);
long get12(const struct S2 *);

#endif

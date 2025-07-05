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
    unsigned short b : 12;
    unsigned int c : 20;
    unsigned int d : 27;
    unsigned long e : 35;
    unsigned long f : 46;
    unsigned long g : 53;
    unsigned long h : 59;
    unsigned long i[2];
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

#endif

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
    unsigned long arr[2];
};

struct S6 {
    unsigned long arr[6];
};

unsigned char fn1(void);
unsigned short fn2(void);
unsigned int fn3(void);
unsigned long fn4(void);
struct S2 fn5(void);
struct S6 fn6(void);

unsigned char test1(void);
unsigned short test2(void);
unsigned int test3(void);
unsigned long test4(void);
struct S2 test5(void);
struct S6 test6(void);

#endif

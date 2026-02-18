/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

extern int global1;

int fn(int);

int test1(int *);
int test2(volatile int *);
int test3(_Atomic int *);
int test4(int);
int test5(int);
int test6(int);
int test7(_Atomic int *);
int test8(_Atomic int *);
int test9(int *);
int test10(int *);
int test11(volatile int *);
int test12(int *, _Atomic int *);

#endif

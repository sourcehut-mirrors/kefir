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
    unsigned long arr[2];
};

struct S1 test1(_Bool, long x);
struct S1 test2(_Bool, unsigned long x);
_Bool test3(_Bool, struct S1);
struct S1 test4(_Bool, struct S1);
struct S1 test5(_Bool, struct S1);
_Bool test6(_Bool, struct S1);
int test7(_Bool, struct S1);
int test8(_Bool, struct S1);
int test9(_Bool, struct S1);
int test10(_Bool, struct S1);
int test11(_Bool, struct S1);
int test12(_Bool, struct S1);

#endif

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

struct i128 {
    unsigned long arr[2];
};

int test1(struct i128, int, int);
int test2(struct i128, int, int);
int test3(struct i128, int, int);
int test4(struct i128, int, int);

struct i128 fact1(struct i128);
struct i128 fact2(struct i128);
struct i128 fact3(struct i128);
struct i128 fact4(struct i128);
struct i128 fact5(struct i128);
struct i128 fact6(struct i128);

struct i128 select1(struct i128);
struct i128 select2(struct i128);

#endif

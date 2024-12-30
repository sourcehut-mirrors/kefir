/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

struct Test1 {
    long x;
    int arr[16];
};

struct Test2 {
    char a;
    struct Test1 b[2];
};

struct Test3 {
    struct Test2 w;
    struct Test1 v;
};

static struct Test1 t1 = {0};
struct Test2 t2 = {0};
struct Test3 t3 = {0};

extern int *arr1 = t1.arr;
extern int *arr2 = t2.b[1].arr;
extern int *arr3 = (&t3.w)->b[0].arr;
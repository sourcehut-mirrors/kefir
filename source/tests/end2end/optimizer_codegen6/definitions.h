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

struct Test1 {
    char x;
    int y;
    short z;
};

int sum1(struct Test1);

struct Test2 {
    _Alignas(16) int x;
    _Alignas(long) char y;
};

int sum2(struct Test2);

struct Test3 {
    int x;
    float y;
    long z;
};

long sum3(struct Test3);

long sum4(long, long, long, long, long, long, long, long, long);

struct Test4 {
    long x;
    long y;
};

struct Test5 {
    long arr[4];
};

long sum5(long, struct Test4, long, struct Test5, long, struct Test4);

#endif

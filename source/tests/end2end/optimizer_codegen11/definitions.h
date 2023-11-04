/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

struct Struct1 {
    int a;
    char buf[10];
    long b;
    short buf2[6];
    char c;
    _Alignas(16) short d;
};

struct Struct1 get_struct1(void);
void zero_struct1(struct Struct1 *);

struct Struct2 {
    long a;
    long b;
};

struct Struct2 get_struct2(long);
void zero_struct2(struct Struct2 *);

#endif

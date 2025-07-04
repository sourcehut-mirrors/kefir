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
    unsigned int e : 25;
};

long get2(struct S1);
long get3(struct S1);
long get4(struct S1);
long get6(struct S1);

struct S2 {
    unsigned long arr[3];
};

long get5(struct S2);
long get7(struct S2);

#endif

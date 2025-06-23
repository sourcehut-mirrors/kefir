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

unsigned char mul1(unsigned char, unsigned char);
unsigned short mul2(unsigned short, unsigned short);
unsigned int mul3(unsigned int, unsigned int);
unsigned long mul4(unsigned long, unsigned long);

struct S2 {
    unsigned long arr[2];
};

struct S2 mul5(struct S2, struct S2);

struct S5 {
    unsigned long arr[5];
};

struct S5 mul6(struct S5, struct S5);

#endif

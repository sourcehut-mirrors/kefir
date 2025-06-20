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

_Bool bnot1(unsigned char);
_Bool bnot2(unsigned short);
_Bool bnot3(unsigned int);
_Bool bnot4(unsigned long);

struct S2 {
    unsigned long arr[2];
};

_Bool bnot5(struct S2);

struct S5 {
    unsigned long arr[5];
};

_Bool bnot6(struct S5);

#endif

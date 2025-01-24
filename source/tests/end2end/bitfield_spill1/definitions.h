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

struct S0 {
    const signed f0 : 2;
    const volatile signed f1 : 15;
    signed f2 : 21;
    signed f3 : 13;
    unsigned f4 : 8;
    volatile signed f5 : 31;
    const unsigned f6 : 1;
    unsigned f7 : 18;
    signed f8 : 14;
};

extern struct S0 x;

#endif

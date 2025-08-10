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

#include "./definitions.h"

long run_op(unsigned long op, long neg, long x, long y) {
entry:
    static void *branches[] = {&&add, &&sub, &&mul, &&div, &&ret};
    long res = 0;
    goto *branches[op % (sizeof(branches) / sizeof(branches[0]))];

add:
    res += x + y;
    goto ret;

sub:
    res += x - y;
    goto ret;

mul:
    res += x * y;
    goto ret;

div:
    res += x / y;
    goto ret;

ret:
    return neg ? -res : res;
}

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

unsigned long test2(struct S2 x) {
    return x.arr[0] + x.arr[1];
}

unsigned long test8(struct S8 x) {
    return x.arr[0] + x.arr[1] + x.arr[2] + x.arr[3] +
        x.arr[4] + x.arr[5] + x.arr[6] + x.arr[7];
}

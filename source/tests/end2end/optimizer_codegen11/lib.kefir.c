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

#include "./definitions.h"

struct Struct1 get_struct1(void) {
    return (struct Struct1){0};
}

void zero_struct1(struct Struct1 *x) {
    *x = (struct Struct1){0};
}

struct Struct2 get_struct2(long a) {
    struct Struct2 x;
    x.a = a;
    x.b = ~a;
    return x;
}

void zero_struct2(struct Struct2 *x) {
    *x = (struct Struct2){0};
}

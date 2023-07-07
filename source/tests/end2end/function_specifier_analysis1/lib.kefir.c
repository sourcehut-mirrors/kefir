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

#include "./definitions.h"

enum Enum1 {
    ONE = 1,
    TWO
} getone(void) {
    return ONE;
}

enum Enum1 gettwo(void) {
    return TWO;
}

enum Enum2 getthree(void) {
    return THREE;
}

enum Enum2 getfour(void) {
    return FOUR;
}

DEFSTRUCT(Struct1) dummyfn(void) {}

int getsz(void) {
    return sizeof(struct Struct1);
}

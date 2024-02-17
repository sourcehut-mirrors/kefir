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

#define DEF(_id, _type)                          \
    void set_##_id(struct S *str, _type value) { \
        str->_id = value;                        \
    }

DEF(i8, char)
DEF(i16, short)
DEF(i32, int)
DEF(i64, long)
DEF(f32, float)
DEF(f64, double)
DEF(a1, struct X)

#undef DEF

void set_arr1_el(struct S *str, int idx, long value) {
    str->arr1[idx] = value;
}

void set_arr2_el(struct S *str, int idx, struct X value) {
    str->arr2[idx] = value;
}

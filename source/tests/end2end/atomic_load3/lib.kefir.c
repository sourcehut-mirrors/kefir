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

char get_i8(const struct S *str) {
    return str->i8;
}

short get_i16(const struct S *str) {
    return str->i16;
}

int get_i32(const struct S *str) {
    return str->i32;
}

long get_i64(const struct S *str) {
    return str->i64;
}

float get_f32(const struct S *str) {
    return str->f32;
}

double get_f64(const struct S *str) {
    return str->f64;
}

struct X get_a1(const struct S *str) {
    return str->a1;
}

long get_arr1_el(const struct S *str, int idx) {
    return str->arr1[idx];
}

struct X get_arr2_el(const struct S *str, int idx) {
    return str->arr2[idx];
}

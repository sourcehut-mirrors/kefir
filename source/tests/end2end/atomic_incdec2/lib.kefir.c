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

char postinc_i8(void) {
    return i8++;
}

short postinc_i16(void) {
    return i16++;
}

int postinc_i32(void) {
    return i32++;
}

long postinc_i64(void) {
    return i64++;
}

long *postinc_ptr(void) {
    return ptr++;
}

float postinc_f32(void) {
    return f32++;
}

double postinc_f64(void) {
    return f64++;
}

long double postinc_ld(void) {
    return ld++;
}

char postdec_i8(void) {
    return i8--;
}

short postdec_i16(void) {
    return i16--;
}

int postdec_i32(void) {
    return i32--;
}

long postdec_i64(void) {
    return i64--;
}

long *postdec_ptr(void) {
    return ptr--;
}

float postdec_f32(void) {
    return f32--;
}

double postdec_f64(void) {
    return f64--;
}

long double postdec_ld(void) {
    return ld--;
}

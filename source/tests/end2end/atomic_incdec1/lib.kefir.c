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

char preinc_i8(void) {
    return ++i8;
}

short preinc_i16(void) {
    return ++i16;
}

int preinc_i32(void) {
    return ++i32;
}

long preinc_i64(void) {
    return ++i64;
}

long *preinc_ptr(void) {
    return ++ptr;
}

float preinc_f32(void) {
    return ++f32;
}

double preinc_f64(void) {
    return ++f64;
}

long double preinc_ld(void) {
    return ++ld;
}

char predec_i8(void) {
    return --i8;
}

short predec_i16(void) {
    return --i16;
}

int predec_i32(void) {
    return --i32;
}

long predec_i64(void) {
    return --i64;
}

long *predec_ptr(void) {
    return --ptr;
}

float predec_f32(void) {
    return --f32;
}

double predec_f64(void) {
    return --f64;
}

long double predec_ld(void) {
    return --ld;
}

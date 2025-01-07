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

struct X {
    long buf[8];
};

struct S {
    _Atomic char i8;
    _Atomic short i16;
    _Atomic int i32;
    _Atomic long i64;
    _Atomic float f32;
    _Atomic double f64;
    _Atomic struct X a1;
    _Atomic long arr1[8];
    _Atomic struct X arr2[2];
};

char get_i8(const struct S *);
short get_i16(const struct S *);
int get_i32(const struct S *);
long get_i64(const struct S *);
float get_f32(const struct S *);
double get_f64(const struct S *);
struct X get_a1(const struct S *);
long get_arr1_el(const struct S *, int);
struct X get_arr2_el(const struct S *, int);

#endif

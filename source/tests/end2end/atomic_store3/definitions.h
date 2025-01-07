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

#define DECL(_id, _type) void set_##_id(struct S *, _type)

DECL(i8, char);
DECL(i16, short);
DECL(i32, int);
DECL(i64, long);
DECL(f32, float);
DECL(f64, double);
DECL(a1, struct X);

#undef DECL

void set_arr1_el(struct S *, int, long);
void set_arr2_el(struct S *, int, struct X);

#endif

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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#define X 123.04
#define Y (28.81 * I)
#define U 2716.21
#define V (62.113 * I)

#define FORMULA1 2 * X + Y / 3 - U / 10 + V
#define FORMULA2 ((long) X + (long) Y) / V *(U - Y)
#define FORMULA3 ((COND(X) ? Y : V) + (X == X) * (V != X) - (U == V)) / (X * Y * U * V)
#define FORMULA4 ((float) X + (float) Y) / (V * (float) -U)

extern _Complex float test1_f32;
extern _Complex double test1_f64;
extern _Complex long double test1_ld;

extern _Complex float test2_f32;
extern _Complex double test2_f64;
extern _Complex long double test2_ld;

extern _Complex float test3_f32;
extern _Complex double test3_f64;
extern _Complex long double test3_ld;

extern _Complex float test4_f32;
extern _Complex double test4_f64;
extern _Complex long double test4_ld;

#endif

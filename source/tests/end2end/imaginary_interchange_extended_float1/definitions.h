/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

extern int f32_size;
extern int f32_alignment;
extern int f32x_size;
extern int f32x_alignment;
extern int f64_size;
extern int f64_alignment;
extern int f64x_size;
extern int f64x_alignment;
extern int f80_size;
extern int f80_alignment;

extern int compat[];

extern float f32_const;
extern double f32x_const;
extern double f64_const;
extern long double f64x_const;
extern long double f80_const;

_Complex float f32_op(int, float, float);
_Complex double f32x_op(int, double, double);
_Complex double f64_op(int, double, double);
_Complex long double f64x_op(int, long double, long double);
_Complex long double f80_op(int, long double, long double);

#endif

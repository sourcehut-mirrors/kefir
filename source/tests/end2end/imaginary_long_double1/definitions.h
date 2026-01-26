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
extern int f80_size;
extern int f80_alignment;
extern const long double *f80_const_ptr;
extern int f80_compat[];

extern long f80_iarr[];
extern long double f80_arr[];
extern long double f80_ops[];
extern double f80_ops2[];

float fi80_to_f32(long double);
double fi80_to_f64(long double);
long double fi80_to_f80(long double);
float fi80_to_fi32(long double);
double fi80_to_fi64(long double);
long double fi80_to_fi80(long double);
float f80_to_fi32(long double);
double f80_to_fi64(long double);
long double f80_to_fi80(long double);

_Complex float fi80_to_cf32(long double);
_Complex double fi80_to_cf64(long double);
_Complex long double fi80_to_cf80(long double);
long fi80_to_i64(long double);
unsigned long fi80_to_u64(long double);
long double i64_to_fi80(long);
long double u64_to_fi80(unsigned long);

long double fi80_neg(long double);
long double fi80_add(long double, long double);
long double fi80_sub(long double, long double);
double fi80_mul(long double, long double);
double fi80_div(long double, long double);

_Complex long double fi80_f80_add(long double, long double);
_Complex long double fi80_f80_sub(long double, long double);
long double fi80_f80_mul(long double, long double);
long double fi80_f80_div(long double, long double);

_Complex long double f80_fi80_add(long double, long double);
_Complex long double f80_fi80_sub(long double, long double);
long double f80_fi80_mul(double, long double);
long double f80_fi80_div(double, long double);

_Complex long double cf80_fi80_add(_Complex double, long double);
_Complex long double cf80_fi80_sub(_Complex double, long double);
_Complex long double cf80_fi80_mul(_Complex double, long double);
_Complex long double cf80_fi80_div(_Complex double, long double);

_Complex long double fi80_cf80_add(long double, _Complex double);
_Complex long double fi80_cf80_sub(long double, _Complex double);
_Complex long double fi80_cf80_mul(long double, _Complex double);
_Complex long double fi80_cf80_div(long double, _Complex double);

_Bool fi80_to_bool(long double);

long double cf32_to_fi80(_Complex float);
long double cf64_to_fi80(_Complex double);
long double cf80_to_fi80(_Complex long double);

#endif

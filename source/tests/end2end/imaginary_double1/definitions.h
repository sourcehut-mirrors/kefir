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
extern int f64_size;
extern int f64_alignment;
extern const double *f64_const_ptr;
extern int f64_compat[];

extern long f64_iarr[];
extern double f64_arr[];
extern double f64_ops[];
extern double f64_ops2[];

float fi64_to_f32(double);
double fi64_to_f64(double);
long double fi64_to_f80(double);
float fi64_to_fi32(double);
double fi64_to_fi64(double);
long double fi64_to_fi80(double);
float f64_to_fi32(double);
double f64_to_fi64(double);
long double f64_to_fi80(double);
_Complex float fi64_to_cf32(double);
_Complex double fi64_to_cf64(double);
_Complex long double fi64_to_cf80(double);
long fi64_to_i64(double);
unsigned long fi64_to_u64(double);
double i64_to_fi64(long);
double u64_to_fi64(unsigned long);

double fi64_neg(double);
double fi64_add(double, double);
double fi64_sub(double, double);
double fi64_mul(double, double);
double fi64_div(double, double);

_Complex double fi64_f64_add(double, double);
_Complex double fi64_f64_sub(double, double);
double fi64_f64_mul(double, double);
double fi64_f64_div(double, double);

_Complex double f64_fi64_add(double, double);
_Complex double f64_fi64_sub(double, double);
double f64_fi64_mul(double, double);
double f64_fi64_div(double, double);

_Complex double cf64_fi64_add(_Complex double, double);
_Complex double cf64_fi64_sub(_Complex double, double);
_Complex double cf64_fi64_mul(_Complex double, double);
_Complex double cf64_fi64_div(_Complex double, double);

_Complex double fi64_cf64_add(double, _Complex double);
_Complex double fi64_cf64_sub(double, _Complex double);
_Complex double fi64_cf64_mul(double, _Complex double);
_Complex double fi64_cf64_div(double, _Complex double);

_Bool fi64_to_bool(double);

#endif

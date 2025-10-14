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

extern int f32_size;
extern int f32_alignment;
extern const float *f32_const_ptr;
extern int f32_compat[];
extern float f32_arr[];
extern float f32_arr2[];
extern _Complex float f32_arr3[];
extern float f32_arr4[];
extern _Complex float f32_arr5[];
extern double f64_arr2[];
extern _Complex double f64_arr3[];
extern double f64_arr4[];
extern _Complex double f64_arr5[];
extern double f64_arr6[];
extern _Complex double f64_arr7[];
extern long double f80_arr2[];
extern _Complex long double f80_arr3[];
extern long double f80_arr4[];
extern _Complex long double f80_arr5[];
extern long double f80_arr6[];
extern _Complex long double f80_arr7[];
extern float f32_ops[];
extern float f32_ops2[];

float fi32_to_f32(float);
double fi32_to_f64(float);
long double fi32_to_f80(float);

float fi32_to_fi32(float);
double fi32_to_fi64(float);
long double fi32_to_fi80(float);

float f32_to_fi32(float);
double f32_to_fi64(float);
long double f32_to_fi80(float);

_Complex float fi32_to_cf32(float);
_Complex double fi32_to_cf64(float);
_Complex long double fi32_to_cf80(float);

long fi32_to_i64(float);
unsigned long fi32_to_u64(float);
float i64_to_fi32(long);
float u64_to_fi32(unsigned long);

float fi32_neg(float);
float fi32_add(float, float);
float fi32_sub(float, float);
float fi32_mul(float, float);
float fi32_div(float, float);

_Complex float fi32_f32_add(float, float);
_Complex float fi32_f32_sub(float, float);
float fi32_f32_mul(float, float);
float fi32_f32_div(float, float);
_Complex float f32_fi32_add(float, float);
_Complex float f32_fi32_sub(float, float);
float f32_fi32_mul(float, float);
float f32_fi32_div(float, float);

_Complex float cf32_fi32_add(_Complex float, float);
_Complex float cf32_fi32_sub(_Complex float, float);
_Complex float cf32_fi32_mul(_Complex float, float);
_Complex float cf32_fi32_div(_Complex float, float);
_Complex float fi32_cf32_add(float, _Complex float);
_Complex float fi32_cf32_sub(float, _Complex float);
_Complex float fi32_cf32_mul(float, _Complex float);
_Complex float fi32_cf32_div(float, _Complex float);

#endif

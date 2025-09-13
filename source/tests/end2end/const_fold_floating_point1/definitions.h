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

float neg32(void);
double neg64(void);
long double neg80(void);
float add32(void);
double add64(void);
long double add80(void);
float sub32(void);
double sub64(void);
long double sub80(void);
float mul32(void);
double mul64(void);
long double mul80(void);
float div32(void);
double div64(void);
long double div80(void);
long to_int_32(void);
long to_int_64(void);
long to_int_80(void);
unsigned long to_uint_32(void);
unsigned long to_uint_64(void);
unsigned long to_uint_80(void);
float int_to_float32(void);
double int_to_float64(void);
long double int_to_float80(void);
float uint_to_float32(void);
double uint_to_float64(void);
long double uint_to_float80(void);
double float32_to_float64(void);
long double float32_to_float80(void);
float float64_to_float32(void);
long double float64_to_float80(void);
float float80_to_float32(void);
double float80_to_float64(void);
int float32_eq_float32(void);
int float64_eq_float64(void);
int float80_eq_float80(void);
int float32_gt_float32(void);
int float64_gt_float64(void);
int float80_gt_float80(void);
int float32_lt_float32(void);
int float64_lt_float64(void);
int float80_lt_float80(void);
int float32_eq2_float32(void);
int float64_eq2_float64(void);
int float80_eq2_float80(void);
int float32_gt2_float32(void);
int float64_gt2_float64(void);
int float80_gt2_float80(void);
int float32_lt2_float32(void);
int float64_lt2_float64(void);
int float80_lt2_float80(void);

#endif

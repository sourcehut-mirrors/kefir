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

float neg32() {
    return -3.14159f;
}

double neg64() {
    return -3.14159e1;
}

long double neg80() {
    return -3.14159e2L;
}

float add32() {
    return 3.14159f + 2.71828f;
}

double add64() {
    return 3.14159e1 + 2.71828e-1;
}

long double add80() {
    return 3.14159e-2L + 2.71828e2L;
}

float sub32() {
    return 3.14159f - 2.71828f;
}

double sub64() {
    return 3.14159e1 - 2.71828e-1;
}

long double sub80() {
    return 3.14159e-2L - 2.71828e2L;
}

float mul32() {
    return 3.14159e1 * 2.71828e-1;
}

double mul64() {
    return 3.14159e-2l * 2.71828e2l;
}

long double mul80() {
    return 3.14159L * 2.71828L;
}

float div32() {
    return 3.14159f / 2.71828f;
}

double div64() {
    return 3.14159e1 / 2.71828e-1;
}

long double div80() {
    return 3.14159e-2L / 2.71828e2L;
}

long to_int_32() {
    return (long) -3.14159f;
}

long to_int_64() {
    return (long) -3.14159e1;
}

long to_int_80() {
    return (long) -3.14159e2;
}

unsigned long to_uint_32() {
    return (unsigned long) 3.14159f;
}

unsigned long to_uint_64() {
    return (unsigned long) 3.14159e1;
}

unsigned long to_uint_80() {
    return (unsigned long) 3.14159e2;
}

float int_to_float32() {
    return (float) -314159;
}

double int_to_float64() {
    return (double) -31415926;
}

long double int_to_float80() {
    return (long double) -31415926;
}

float uint_to_float32() {
    return (float) 314159u;
}

double uint_to_float64() {
    return (double) 31415926u;
}

long double uint_to_float80() {
    return (long double) 31415926u;
}

double float32_to_float64() {
    return 3.14159f;
}

long double float32_to_float80() {
    return 3.14159e1f;
}

float float64_to_float32() {
    return 3.14159e2;
}

long double float64_to_float80() {
    return 2.71828e3;
}

float float80_to_float32() {
    return 2.71828e4l;
}

double float80_to_float64() {
    return 2.71828e5l;
}

int float32_eq_float32() {
    return 12.347f == 12.345f;
}

int float64_eq_float64() {
    return 12.348 == 12.345;
}

int float80_eq_float80() {
    return 12.349L == 12.345L;
}

int float32_eq2_float32() {
    return 12.347f == 12.347f;
}

int float64_eq2_float64() {
    return 12.346 == 12.346;
}

int float80_eq2_float80() {
    return 12.345L == 12.345L;
}

int float32_gt_float32() {
    return 12.346f > 12.345f;
}

int float64_gt_float64() {
    return 12.346 > 12.345;
}

int float80_gt_float80() {
    return 12.346L > 12.345L;
}

int float32_gt2_float32() {
    return -12.346f > 12.345f;
}

int float64_gt2_float64() {
    return -12.346 > 12.345;
}

int float80_gt2_float80() {
    return -12.346L > 12.345L;
}

int float32_lt_float32() {
    return 12.346f < 12.347f;
}

int float64_lt_float64() {
    return 12.346 < 12.347;
}

int float80_lt_float80() {
    return 12.346L < 12.347L;
}

int float32_lt2_float32() {
    return 12.346f < -12.347f;
}

int float64_lt2_float64() {
    return 12.346 < -12.347;
}

int float80_lt2_float80() {
    return 12.346L < -12.347L;
}

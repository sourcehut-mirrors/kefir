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

float PI_float(void) {
    return PI_F;
}

double PI_double(void) {
    return PI_D;
}

float E_float(void) {
    return E_F;
}

double E_double(void) {
    return E_D;
}

float long_to_float(long x) {
    return (float) x;
}

double long_to_double(long x) {
    return (double) x;
}

float ulong_to_float(unsigned long x) {
    return (float) x;
}

double ulong_to_double(unsigned long x) {
    return (double) x;
}

extern long float_to_long(float x) {
    return (long) x;
}

extern long double_to_long(double x) {
    return (long) x;
}

extern unsigned long float_to_ulong(float x) {
    return (unsigned long) x;
}

extern unsigned long double_to_ulong(double x) {
    return (unsigned long) x;
}

extern double float_to_double(float x) {
    return (double) x;
}

extern float double_to_float(double x) {
    return (float) x;
}

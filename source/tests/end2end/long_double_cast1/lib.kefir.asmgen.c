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

#include "./definitions.h"

long double intcld(int x) {
    return (long double) x;
}

long double uintcld(unsigned int x) {
    return (long double) x;
}

long double f32cld(float x) {
    return (long double) x;
}

long double f64cld(double x) {
    return (long double) x;
}

int ldcint(long double ld) {
    return (int) ld;
}

unsigned int ldcuint(long double ld) {
    return (unsigned int) ld;
}

float ldcf32(long double ld) {
    return (float) ld;
}

double ldcf64(long double ld) {
    return (double) ld;
}

long double neg(const long double x) {
    return -x;
}

long double inc(x)
long double x;
{ return x + 1.0l; }

long double imp_cast(int x) {
    return x;
}

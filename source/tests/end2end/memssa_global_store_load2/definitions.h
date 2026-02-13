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

extern _Thread_local char c1, c2, c3;
extern _Thread_local short s1, s2, s3;
extern _Thread_local int i1, i2, i3;
extern _Thread_local long l1, l2, l3;
extern _Thread_local float f1, f2, f3;
extern _Thread_local double d1, d2, d3;
extern _Thread_local long double ld1, ld2, ld3;
extern _Thread_local _Complex float cf1, cf2, cf3;
extern _Thread_local _Complex double cd1, cd2, cd3;
extern _Thread_local _Complex long double cld1, cld2, cld3;

char test_char(char);
short test_short(short);
int test_int(int);
long test_long(long);
float test_float(float);
double test_double(double);
long double test_ldouble(long double);
_Complex float test_cfloat(_Complex float);
_Complex double test_cdouble(_Complex double);
_Complex long double test_cldouble(_Complex long double);

#endif

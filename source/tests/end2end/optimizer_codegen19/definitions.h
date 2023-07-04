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

#define PI_LD 3.14159L
#define E_LD 2.718281828L

long double get_pi(void);
long double get_e(void);

long double addld(long double, long double);
long double subld(long double, long double);
long double mulld(long double, long double);
long double divld(long double, long double);
long double negld(long double);

int ldequals(long double, long double);
int ldgreater(long double, long double);
int ldlesser(long double, long double);

long double long_to_long_double(long);
long double ulong_to_long_double(unsigned long);
long double float_to_long_double(float);
long double double_to_long_double(double);

long long_double_to_long(long double);
unsigned long long_double_to_ulong(long double);
int long_double_trunc(long double);
_Bool long_double_to_bool(long double);
float long_double_to_float(long double);
double long_double_to_double(long double);

long double my_hypot(long double, long double);

long double sumallld(int, ...);

long double test1(long double, long double);
long double test2(long double, long double);
long double test3(long double, long double);
long double test4(long double, long double);
long double test5(long double, long double);
long double test6(long double, long double);

#endif

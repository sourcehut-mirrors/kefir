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

int less_thanf(float, float);
int less_than(double, double);
int less_thanl(long double, long double);

int eq_less_thanf(float, float);
int eq_less_than(double, double);
int eq_less_thanl(long double, long double);

int eqf(float, float);
int eq(double, double);
int eql(long double, long double);

int neqf(float, float);
int neq(double, double);
int neql(long double, long double);

int greater_thanf(float, float);
int greater_than(double, double);
int greater_thanl(long double, long double);

int eq_greater_thanf(float, float);
int eq_greater_than(double, double);
int eq_greater_thanl(long double, long double);

#endif

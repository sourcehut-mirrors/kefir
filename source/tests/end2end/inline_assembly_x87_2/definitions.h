/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

#ifdef __x86_64__
extern float subf(float, float);
extern double sub(double, double);
extern long double subl(long double, long double);

extern float mydivf(float, float);
extern double mydiv(double, double);
extern long double mydivl(long double, long double);

extern float mydivrf(float, float);
extern double mydivr(double, double);
extern long double mydivrl(long double, long double);
#endif

#endif

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

#define PI_F 3.14159f
#define E_F 2.71828f
#define PI_D 3.14159
#define E_D 2.71828

extern float PI_float(void);
extern double PI_double(void);

extern float E_float(void);
extern double E_double(void);

extern float long_to_float(long);
extern double long_to_double(long);

extern float ulong_to_float(unsigned long);
extern double ulong_to_double(unsigned long);

extern long float_to_long(float);
extern long double_to_long(double);

extern unsigned long float_to_ulong(float);
extern unsigned long double_to_ulong(double);

extern double float_to_double(float);
extern float double_to_float(double);

#endif

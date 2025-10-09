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

extern int f32x_size;
extern int f32x_alignment;
extern const double *f32x_const_ptr;
extern int f32x_compat[];
extern double f32x[];

double get32x_1(void);
double get32x_2(void);

double neg32x(double);
double add32x(double, double);
double sub32x(double, double);
double mul32x(double, double);
double div32x(double, double);
double conv1(long);
double conv2(unsigned long);
double conv3(float);
double conv4(double);
double conv5(long double);

#endif

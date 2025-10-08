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

extern int f32_size;
extern int f32_alignment;
extern const float *f32_const_ptr;
extern int f32_compat[];
extern float f32[];

float get32_1(void);
float get32_2(void);

float neg32(float);
float add32(float, float);
float sub32(float, float);
float mul32(float, float);
float div32(float, float);
float conv1(long);
float conv2(unsigned long);
float conv3(float);
float conv4(double);
float conv5(long double);

#endif

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

extern int f80_size;
extern int f80_alignment;
extern const long double *f80_const_ptr;
extern int f80_compat[];
extern long double f80[];

long double get80_1(void);
long double get80_2(void);

long double neg80(long double);
long double add80(long double, long double);
long double sub80(long double, long double);
long double mul80(long double, long double);
long double div80(long double, long double);
long double conv1(long);
long double conv2(unsigned long);
long double conv3(float);
long double conv4(double);
long double conv5(long double);

#endif

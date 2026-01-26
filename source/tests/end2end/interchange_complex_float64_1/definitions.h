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

extern int f64_size;
extern int f64_alignment;
extern const _Complex double *f64_const_ptr;
extern int f64_compat[];
extern _Complex double f64[];

_Complex double get64_1(void);
_Complex double get64_2(void);

_Complex double neg64(_Complex double);
_Complex double add64(_Complex double, _Complex double);
_Complex double sub64(_Complex double, _Complex double);
_Complex double mul64(_Complex double, _Complex double);
_Complex double div64(_Complex double, _Complex double);
_Complex double conv1(long);
_Complex double conv2(unsigned long);
_Complex double conv3(float);
_Complex double conv4(double);
_Complex double conv5(long double);
_Complex double conv6(_Complex float);
_Complex double conv7(_Complex double);
_Complex double conv8(_Complex long double);

#endif

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

extern int f80_size;
extern int f80_alignment;
extern const _Complex long double *f80_const_ptr;
extern int f80_compat[];
extern _Complex long double f80[];

_Complex long double get80_1(void);
_Complex long double get80_2(void);

_Complex long double neg80(_Complex long double);
_Complex long double add80(_Complex long double, _Complex long double);
_Complex long double sub80(_Complex long double, _Complex long double);
_Complex long double mul80(_Complex long double, _Complex long double);
_Complex long double div80(_Complex long double, _Complex long double);
_Complex long double conv1(long);
_Complex long double conv2(unsigned long);
_Complex long double conv3(float);
_Complex long double conv4(double);
_Complex long double conv5(long double);
_Complex long double conv6(_Complex float);
_Complex long double conv7(_Complex double);
_Complex long double conv8(_Complex long double);

#endif

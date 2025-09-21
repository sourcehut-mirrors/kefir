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

extern _Atomic _Complex float f32_1;
extern _Atomic _Complex double f64_1;
extern _Atomic _Complex long double ld_1;

_Complex float add_f32(_Atomic const _Complex float *);
_Complex float sub_f32(_Atomic const _Complex float *);
_Complex float mul_f32(_Atomic const _Complex float *);
_Complex float div_f32(_Atomic const _Complex float *);
_Complex float neg_f32(void);

_Complex double add_f64(_Atomic const _Complex double *);
_Complex double sub_f64(_Atomic const _Complex double *);
_Complex double mul_f64(_Atomic const _Complex double *);
_Complex double div_f64(_Atomic const _Complex double *);
_Complex double neg_f64(void);

_Complex long double add_ld(_Atomic const _Complex long double *);
_Complex long double sub_ld(_Atomic const _Complex long double *);
_Complex long double mul_ld(_Atomic const _Complex long double *);
_Complex long double div_ld(_Atomic const _Complex long double *);
_Complex long double neg_ld(void);

#endif

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

_Complex float cfloat_new(float);
_Complex double cdouble_new(double);
_Complex long double cldouble_new(long double);

_Complex float cfloat_from_long(long);
_Complex float cfloat_from_double(double);
_Complex float cfloat_from_ldouble(long double);

_Complex double cdouble_from_long(long);
_Complex double cdouble_from_float(float);
_Complex double cdouble_from_ldouble(long double);

_Complex long double cldouble_from_long(long);
_Complex long double cldouble_from_float(float);
_Complex long double cldouble_from_double(double);

_Complex float cfloat_from_cfloat(_Complex float);
_Complex float cfloat_from_cdouble(_Complex double);
_Complex float cfloat_from_cldouble(_Complex long double);

_Complex double cdouble_from_cfloat(_Complex float);
_Complex double cdouble_from_cdouble(_Complex double);
_Complex double cdouble_from_cldouble(_Complex long double);

_Complex long double cldouble_from_cfloat(_Complex float);
_Complex long double cldouble_from_cdouble(_Complex double);
_Complex long double cldouble_from_cldouble(_Complex long double);

long long_from_cfloat(_Complex float);
unsigned long ulong_from_cfloat(_Complex float);

long long_from_cdouble(_Complex double);
unsigned long ulong_from_cdouble(_Complex double);

long long_from_cldouble(_Complex long double);
unsigned long ulong_from_cldouble(_Complex long double);

_Bool bool_from_cfloat(_Complex float);
_Bool bool_from_cdouble(_Complex double);
_Bool bool_from_cldouble(_Complex long double);

#endif

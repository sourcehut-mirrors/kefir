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

_Complex float cmpf32add(_Complex float, _Complex float);
_Complex double cmpf64add(_Complex double, _Complex double);
_Complex long double cmpldadd(_Complex long double, _Complex long double);

_Complex float cmpf32sub(_Complex float, _Complex float);
_Complex double cmpf64sub(_Complex double, _Complex double);
_Complex long double cmpldsub(_Complex long double, _Complex long double);

_Complex float cmpf32mul(_Complex float, _Complex float);
_Complex double cmpf64mul(_Complex double, _Complex double);
_Complex long double cmpldmul(_Complex long double, _Complex long double);

_Complex float cmpf32div(_Complex float, _Complex float);
_Complex double cmpf64div(_Complex double, _Complex double);
_Complex long double cmplddiv(_Complex long double, _Complex long double);

_Complex float cmpf32neg(_Complex float);
_Complex double cmpf64neg(_Complex double);
_Complex long double cmpldneg(_Complex long double);

#endif

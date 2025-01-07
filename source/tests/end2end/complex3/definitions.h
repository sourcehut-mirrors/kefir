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

int cmpf32(_Complex float, _Complex float);
int cmpf64(_Complex double, _Complex double);
int cmpld(_Complex long double, _Complex long double);

int cmpf32_not(_Complex float, _Complex float);
int cmpf64_not(_Complex double, _Complex double);
int cmpld_not(_Complex long double, _Complex long double);

_Bool cmpf32_bool(_Complex float);
_Bool cmpf64_bool(_Complex double);
_Bool cmpld_bool(_Complex long double);

#endif

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

#include "./definitions.h"

float my_nanf = __builtin_nanf("");
double my_nand = __builtin_nan("");
long double my_nanld = __builtin_nanl("");

float my_nanf2(void) {
    return __builtin_nanf("");
}

double my_nand2(void) {
    return __builtin_nan("");
}

long double my_nanld2(void) {
    return __builtin_nanl("");
}

/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

float my_inff(void) {
    return __builtin_inff();
}

double my_inf(void) {
    return __builtin_inf();
}

long double my_infl(void) {
    return __builtin_infl();
}

float my_huge_valf(void) {
    return __builtin_huge_valf();
}

double my_huge_val(void) {
    return __builtin_huge_val();
}

long double my_huge_vall(void) {
    return __builtin_huge_vall();
}
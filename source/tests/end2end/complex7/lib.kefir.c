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

_Complex long double test1(void) {
    return 1.0fi + 1.0Fi + 1.0if + 1.0iF + 1.0fI + 1.0FI + 1.0If + 1.0IF + 1.0i + 1.0I + 1.0li + 1.0Li + 1.0il + 1.0iL +
           1.0lI + 1.0LI + 1.0Il + 1.0IL;
}

_Complex float cmpf32(float a, float b) {
    return a + b * 1.0fi;
}

_Complex double cmpf64(double a, double b) {
    return a + b * 1.0i;
}

_Complex long double cmpld(long double a, long double b) {
    return a + b * 1.0li;
}

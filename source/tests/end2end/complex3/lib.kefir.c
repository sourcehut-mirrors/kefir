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

int cmpf32(_Complex float x, _Complex float y) {
    return x == y;
}

int cmpf64(_Complex double x, _Complex double y) {
    return x == y;
}

int cmpf32_not(_Complex float x, _Complex float y) {
    return x != y;
}

int cmpf64_not(_Complex double x, _Complex double y) {
    return x != y;
}

int cmpld(_Complex long double x, _Complex long double y) {
    return x == y;
}

int cmpld_not(_Complex long double x, _Complex long double y) {
    return x != y;
}

_Bool cmpf32_bool(_Complex float x) {
    return x;
}

_Bool cmpf64_bool(_Complex double x) {
    return x;
}

_Bool cmpld_bool(_Complex long double x) {
    return x;
}

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

#include "definitions.h"

static const _BitInt(135) arr[] = {-1891475049610273521994429918325638525836wb,
                                   1891475049610273521994429918325638525836wb};

static const unsigned _BitInt(135) uarr[] = {1891475049610273521994429918325638525836uwb};

float get1() {
    return arr[0];
}

float get2() {
    return arr[1];
}

float get3() {
    return uarr[0];
}

/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include "./definitions.h"

int main() {
    assert(Limits[0] == CHAR_BIT);
    assert(Limits[1] == SCHAR_MAX);
    assert(Limits[2] == SHRT_MAX);
    assert(Limits[3] == INT_MAX);
    assert(Limits[4] == LONG_MAX);
    assert(Limits[5] == LLONG_MAX);
    assert(Limits[6] == sizeof(short) * CHAR_BIT);
    assert(Limits[7] == sizeof(int) * CHAR_BIT);
    assert(Limits[8] == sizeof(long) * CHAR_BIT);
    assert(Limits[9] == sizeof(long long) * CHAR_BIT);
    return EXIT_SUCCESS;
}

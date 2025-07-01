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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include "./definitions.h"

int main(void) {
    assert(fabs(get1() - 314.0f) < 1e-5);
    assert(fabs(get2() - 271.0f) < 1e-5);
    assert(fabs(get3() - 314159.0) < 1e-6);
    assert(fabs(get4() - 271828.0) < 1e-6);
    assert(fabsl(get5() - 31415926.0l) < 1e-7);
    assert(fabsl(get6() - 2718281828.0l) < 1e-7);
    return EXIT_SUCCESS;
}

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
#include <math.h>
#include <complex.h>
#include "./definitions.h"

int main(void) {
    assert(fabs(arr[0] - 3.14159) < 1e-6);
    assert(fabs(arr[1] - 2.718281828) < 1e-6);
    assert(fabs(arr[2] - 3.14159) < 1e-6);
    assert(fabs(arr[3] - 2.718281828) < 1e-6);
    assert(fabs(arr[4] - 0xaabbe.555221P+10) < 1e-6);
    assert(fabs(arr[5] - 0x123498765.9172bedap-1001) < 1e-6);
    assert(fabsl(a - 18721767.18171e34l) < 1e-7);
    assert(fabsl(b - 0xabbeeecccc.6322e32ep-112l) < 1e-7);
    assert(fabsl(crealf(c) - 3.14159f) < 1e-5);
    assert(fabsl(cimagf(c) - 2.71828f) < 1e-5);
    return EXIT_SUCCESS;
}

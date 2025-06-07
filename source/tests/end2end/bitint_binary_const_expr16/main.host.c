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
#include <complex.h>
#include "./definitions.h"

int main(void) {
    assert(bools[0] == 1);
    assert(bools[1] == 0);
    assert(bools[2] == 0);
    assert(bools[3] == 1);
    assert(bools[4] == 1);
    assert(bools[5] == 1);
    assert(bools[6] == 0);

    assert(bools[7] == 1);
    assert(bools[8] == 1);
    assert(bools[9] == 0);
    assert(bools[10] == 1);
    assert(bools[11] == 1);
    assert(bools[12] == 0);
    return EXIT_SUCCESS;
}

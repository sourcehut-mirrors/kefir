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
    assert(cmps[0] == 1);
    assert(cmps[1] == 1);
    assert(cmps[2] == 1);
    assert(cmps[3] == 0);
    assert(cmps[4] == 0);
    assert(cmps[5] == 0);
    assert(cmps[6] == 1);
    assert(cmps[7] == 0);
    assert(cmps[8] == 1);

    assert(cmps[9] == 1);
    assert(cmps[10] == 0);
    assert(cmps[11] == 1);
    assert(cmps[12] == 0);
    assert(cmps[13] == 1);
    assert(cmps[14] == 1);
    assert(cmps[15] == 1);
    assert(cmps[16] == 1);
    assert(cmps[17] == 0);
    assert(cmps[18] == 0);

    assert(cmps[19] == 0);
    assert(cmps[20] == 1);
    assert(cmps[21] == 0);
    assert(cmps[22] == 1);

    assert(cmps[23] == 1);
    assert(cmps[24] == 1);
    assert(cmps[25] == 1);
    assert(cmps[26] == 1);
    assert(cmps[27] == 1);
    assert(cmps[28] == 1);
    return EXIT_SUCCESS;
}

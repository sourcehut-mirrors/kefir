/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
#include <string.h>
#include <stdint.h>
#include "./definitions.h"

int main(void) {
    const long value = INT64_MIN;
    assert(fabs(test32(value) - (float) (value) - (float) (value + 1) - (float) (value + 2) - (float) (value + 3) -
                (float) (value + 4) - (float) (value + 5) - (float) (value + 6) - (float) (value + 7) -
                (float) (value + 8) - (float) (value + 9) - (float) (value + 10) - (float) (value + 11) -
                (float) (value + 12) - (float) (value + 13) - (float) (value + 14) - (float) (value + 15) -
                (float) (value + 16)) < 1e-6);
    assert(fabs(test64(value) - (double) (value) - (double) (value + 1) - (double) (value + 2) - (double) (value + 3) -
                (double) (value + 4) - (double) (value + 5) - (double) (value + 6) - (double) (value + 7) -
                (double) (value + 8) - (double) (value + 9) - (double) (value + 10) - (double) (value + 11) -
                (double) (value + 12) - (double) (value + 13) - (double) (value + 14) - (double) (value + 15) -
                (double) (value + 16)) < 1e-6);
    return EXIT_SUCCESS;
}

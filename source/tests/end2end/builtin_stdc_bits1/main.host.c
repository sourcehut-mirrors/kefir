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
#include "./definitions.h"

int main(void) {
    // As per GCC 15.1.1 output
    assert(bit_ceil(0) == 1);
    assert(bit_ceil(1) == 1);
    assert(bit_ceil(2) == 2);
    assert(bit_ceil(3) == 4);
    assert(bit_ceil(4) == 4);
    assert(bit_ceil(5) == 8);
    assert(bit_ceil(1024) == 1024);

    assert(bit_floor(0) == 0);
    assert(bit_floor(1) == 1);
    assert(bit_floor(2) == 2);
    assert(bit_floor(3) == 2);
    assert(bit_floor(4) == 4);
    assert(bit_floor(5) == 4);
    assert(bit_floor(1024) == 1024);

    assert(rotate_left(1, 1) == 2);
    assert(rotate_left(1, 63) == (1ull << 63));
    assert(rotate_left(1, 64) == 1);
    assert(rotate_left(1023, 10) == 1023 << 10);

    assert(rotate_right(1, 1) == 1ull << 63);
    assert(rotate_right(1024, 3) == 128);
    assert(rotate_right(1ull << 63, 30) == 1ull << 33);
    assert(rotate_right(1ull << 30, 61) == 1ull << 33);
    return EXIT_SUCCESS;
}

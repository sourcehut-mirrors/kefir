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
#include <stddef.h>
#include "./definitions.h"

int main(void) {
    assert(my_ffs(0) == 0);
    assert(my_ffs(1) == 1);
    assert(my_ffs(2) == 2);
    assert(my_ffs(3) == 1);
    assert(my_ffs(4) == 3);
    assert(my_ffs(5) == 1);
    assert(my_ffs(8) == 4);
    assert(my_ffs(16) == 5);
    assert(my_ffs(32) == 6);
    assert(my_ffs(63) == 1);
    assert(my_ffs(64) == 7);

    assert(my_clz(1) == 31);
    assert(my_clz(2) == 30);
    assert(my_clz(3) == 30);
    assert(my_clz(4) == 29);
    assert(my_clz(8) == 28);
    assert(my_clz(16) == 27);
    assert(my_clz(32) == 26);
    assert(my_clz(63) == 26);
    assert(my_clz(64) == 25);
    assert(my_clz(-1) == 0);
    assert(my_clz(-16) == 0);
    assert(my_clz(-128) == 0);

    assert(my_ctz(1) == 0);
    assert(my_ctz(2) == 1);
    assert(my_ctz(3) == 0);
    assert(my_ctz(4) == 2);
    assert(my_ctz(5) == 0);
    assert(my_ctz(8) == 3);
    assert(my_ctz(16) == 4);
    assert(my_ctz(32) == 5);
    assert(my_ctz(63) == 0);
    assert(my_ctz(64) == 6);

    assert(my_clrsb(1) == 30);
    assert(my_clrsb(2) == 29);
    assert(my_clrsb(3) == 29);
    assert(my_clrsb(4) == 28);
    assert(my_clrsb(8) == 27);
    assert(my_clrsb(16) == 26);
    assert(my_clrsb(32) == 25);
    assert(my_clrsb(63) == 25);
    assert(my_clrsb(64) == 24);
    assert(my_clrsb(-1) == 31);
    assert(my_clrsb(-16) == 27);
    assert(my_clrsb(-128) == 24);

    assert(my_popcount(0) == 0);
    assert(my_popcount(1) == 1);
    assert(my_popcount(2) == 1);
    assert(my_popcount(3) == 2);
    assert(my_popcount(4) == 1);
    assert(my_popcount(15) == 4);
    assert(my_popcount(16) == 1);
    assert(my_popcount(-1) == 32);
    assert(my_popcount(-2) == 31);

    assert(my_parity(0) == 0);
    assert(my_parity(1) == 1);
    assert(my_parity(2) == 1);
    assert(my_parity(3) == 0);
    assert(my_parity(4) == 1);
    assert(my_parity(15) == 0);
    assert(my_parity(16) == 1);
    assert(my_parity(-1) == 0);
    assert(my_parity(-2) == 1);
    return EXIT_SUCCESS;
}

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
    assert(int128_ffs((struct i128){{0, 0}}) == 0);
    assert(int128_ffs((struct i128){{1, 0}}) == 1);
    assert(int128_ffs((struct i128){{1024, 0}}) == 11);
    assert(int128_ffs((struct i128){{1ull << 63, 0}}) == 64);
    assert(int128_ffs((struct i128){{~0ull, 0}}) == 1);
    assert(int128_ffs((struct i128){{0, 1}}) == 65);
    assert(int128_ffs((struct i128){{0, 1ull << 63}}) == 128);
    assert(int128_ffs((struct i128){{1, 1ull << 63}}) == 1);
    assert(int128_ffs((struct i128){{0, ~0ull}}) == 65);
    assert(int128_ffs((struct i128){{~0ull, ~0ull}}) == 1);

    assert(int128_clz((struct i128){{0, 0}}, -1000) == -1000);
    assert(int128_clz((struct i128){{1, 0}}, -1000) == 127);
    assert(int128_clz((struct i128){{1024, 0}}, -1000) == 117);
    assert(int128_clz((struct i128){{~0ull, 0}}, -1000) == 64);
    assert(int128_clz((struct i128){{0, 1}}, -1000) == 63);
    assert(int128_clz((struct i128){{0, 1ull << 61}}, -1000) == 2);
    assert(int128_clz((struct i128){{0, 1ull << 62}}, -1000) == 1);
    assert(int128_clz((struct i128){{0, 1ull << 63}}, -1000) == 0);
    assert(int128_clz((struct i128){{~0ull, ~0ull}}, -1000) == 0);

    assert(int128_ctz((struct i128){{0, 0}}, -2000) == -2000);
    assert(int128_ctz((struct i128){{1, 0}}, -2000) == 0);
    assert(int128_ctz((struct i128){{2, 0}}, -2000) == 1);
    assert(int128_ctz((struct i128){{1024, 0}}, -2000) == 10);
    assert(int128_ctz((struct i128){{1ull << 63, 0}}, -2000) == 63);
    assert(int128_ctz((struct i128){{~0ull, 0}}, -2000) == 0);
    assert(int128_ctz((struct i128){{0, 1}}, -2000) == 64);
    assert(int128_ctz((struct i128){{0, 1ull << 32}}, -2000) == 96);
    assert(int128_ctz((struct i128){{0, 1ull << 63}}, -2000) == 127);
    assert(int128_ctz((struct i128){{0, ~0ull}}, -2000) == 64);
    assert(int128_ctz((struct i128){{1, ~0ull}}, -2000) == 0);
    assert(int128_ctz((struct i128){{~0ull, ~0ull}}, -2000) == 0);

    assert(int128_clrsb((struct i128){{0, 0}}) == 127);
    assert(int128_clrsb((struct i128){{1, 0}}) == 126);
    assert(int128_clrsb((struct i128){{1024, 0}}) == 116);
    assert(int128_clrsb((struct i128){{~0ull, 0}}) == 63);
    assert(int128_clrsb((struct i128){{0, 1}}) == 62);
    assert(int128_clrsb((struct i128){{0, (1ull << 62) - 1}}) == 1);
    assert(int128_clrsb((struct i128){{0, (1ull << 63) - 1}}) == 0);
    assert(int128_clrsb((struct i128){{~0ull, ~0ull}}) == 127);
    assert(int128_clrsb((struct i128){{~0ull - 1, ~0ull}}) == 126);
    assert(int128_clrsb((struct i128){{0, ~0ull}}) == 63);
    assert(int128_clrsb((struct i128){{0, (1ull << 63) | (1ull << 62)}}) == 1);
    assert(int128_clrsb((struct i128){{0, 1ull << 63}}) == 0);

    assert(int128_popcount((struct i128){{0, 0}}) == 0);
    assert(int128_popcount((struct i128){{1, 0}}) == 1);
    assert(int128_popcount((struct i128){{1023, 0}}) == 10);
    assert(int128_popcount((struct i128){{1024, 0}}) == 1);
    assert(int128_popcount((struct i128){{1ull << 63, 0}}) == 1);
    assert(int128_popcount((struct i128){{(1ull << 63) - 1, 0}}) == 63);
    assert(int128_popcount((struct i128){{~0ull, 0}}) == 64);
    assert(int128_popcount((struct i128){{0, 1}}) == 1);
    assert(int128_popcount((struct i128){{~0ull, 1}}) == 65);
    assert(int128_popcount((struct i128){{~0ull, 1ull << 63}}) == 65);
    assert(int128_popcount((struct i128){{~0ull, (1ull << 62) - 1}}) == 126);
    assert(int128_popcount((struct i128){{~0ull, (1ull << 63) - 1}}) == 127);
    assert(int128_popcount((struct i128){{~0ull, ~0ull}}) == 128);

    assert(int128_parity((struct i128){{0, 0}}) == 0);
    assert(int128_parity((struct i128){{1, 0}}) == 1);
    assert(int128_parity((struct i128){{1023, 0}}) == 0);
    assert(int128_parity((struct i128){{1024, 0}}) == 1);
    assert(int128_parity((struct i128){{1ull << 63, 0}}) == 1);
    assert(int128_parity((struct i128){{(1ull << 63) - 1, 0}}) == 1);
    assert(int128_parity((struct i128){{~0ull, 0}}) == 0);
    assert(int128_parity((struct i128){{0, 1}}) == 1);
    assert(int128_parity((struct i128){{~0ull, 1}}) == 1);
    assert(int128_parity((struct i128){{~0ull, 1ull << 63}}) == 1);
    assert(int128_parity((struct i128){{~0ull, (1ull << 62) - 1}}) == 0);
    assert(int128_parity((struct i128){{~0ull, (1ull << 63) - 1}}) == 1);
    assert(int128_parity((struct i128){{~0ull, ~0ull}}) == 0);
    return EXIT_SUCCESS;
}

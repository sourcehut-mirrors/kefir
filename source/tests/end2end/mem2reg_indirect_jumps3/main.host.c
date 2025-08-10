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
#include "./definitions.h"

int main(void) {
    for (int i = -100; i <= 100; i++) {
        for (int j = -100; j <= 100; j++) {
            for (int k = 0; k < 3; k++) {
                assert(run_op(5 * k + 0, 0, i, j) == i + j);
                assert(run_op(5 * k + 0, 1, i, j) == -(i + j));
                assert(run_op(5 * k + 1, 0, i, j) == i - j);
                assert(run_op(5 * k + 1, 1, i, j) == -(i - j));
                assert(run_op(5 * k + 2, 0, i, j) == i * j);
                assert(run_op(5 * k + 2, 1, i, j) == -(i * j));
                if (j != 0) {
                    assert(run_op(5 * k + 3, 0, i, j) == i / j);
                    assert(run_op(5 * k + 3, 1, i, j) == -(i / j));
                }
                assert(run_op(5 * k + 4, 0, i, j) == 0);
                assert(run_op(5 * k + 4, 1, i, j) == 0);
            }
        }
    }
    return EXIT_SUCCESS;
}

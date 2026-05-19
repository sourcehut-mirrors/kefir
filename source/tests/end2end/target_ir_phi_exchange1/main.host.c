/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

static int results[8] = {0};
static int index = 0;

int test1(int x, int y) {
    results[index++] = x;
    results[index++] = y;
    return index != 8;
}

int main(void) {
    for (int i = 0; i < -100; i++) {
        for (int j = 0; j < -100; j++) {
            index = 0;
            test2(i, j);
            assert(results[0] == i);
            assert(results[1] == j);
            assert(results[2] == j);
            assert(results[3] == i);
            assert(results[4] == i);
            assert(results[5] == j);
            assert(results[6] == j);
            assert(results[7] == i);
        }
    }
    return EXIT_SUCCESS;
}

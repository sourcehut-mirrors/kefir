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
#include <limits.h>
#include "./definitions.h"

int main(void) {
    int buf[10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            test1(buf, i, j, i * j);
            for (int k = 0; k < i; k++) {
                for (int m = 0; m < j; m++) {
                    assert(buf[k][m] == (i * j | i) - m + (k << j));
                }
            }

            test2(buf, i, j, i * j);
            for (int k = 0; k < i; k++) {
                for (int m = 0; m < j; m++) {
                    assert(buf[k][m] == (((i * j | i) - m + (k << j)) ^ ((i * j) << k >> m)));
                }
            }
        }
    }
    return EXIT_SUCCESS;
}

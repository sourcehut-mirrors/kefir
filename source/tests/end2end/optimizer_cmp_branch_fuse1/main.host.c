/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

void fn1(void) {}

int main(void) {
    for (int i = -100; i <= 100; i++) {
        unsigned int ui = (unsigned int) (i + 100);

        for (int j = -100; j <= 100; j++) {
            unsigned int uj = (unsigned int) (j + 100);

            assert(equals(i, j) == (i == j ? 1 : 0));
            assert(not_equals(i, j) == (i != j ? 1 : 0));
            assert(not_equals2(i, j) == (i != j ? 1 : 0));
            assert(greater(i, j) == (i > j ? 1 : 0));
            assert(not_greater(i, j) == (i <= j ? 1 : 0));
            assert(less(i, j) == (i < j ? 1 : 0));
            assert(not_less(i, j) == (i >= j ? 1 : 0));
            assert(above(ui, uj) == (ui > uj ? 1 : 0));
            assert(not_above(ui, uj) == (ui <= uj ? 1 : 0));
            assert(below(ui, uj) == (ui < uj ? 1 : 0));
            assert(not_below(ui, uj) == (ui >= uj ? 1 : 0));
        }
    }
    return EXIT_SUCCESS;
}

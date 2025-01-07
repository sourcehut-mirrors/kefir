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
        unsigned int ui = (unsigned int) (i + 100);
        for (int j = -100; j <= 100; j++) {
            unsigned int uj = (unsigned int) (j + 100);

            assert(greater_or_equals(i, j) == (i >= j ? 1 : 0));
            assert(greater_or_equals2(i, j) == (i >= j ? 1 : 0));
            assert(less_or_equals(i, j) == (i <= j ? 1 : 0));
            assert(less_or_equals2(i, j) == (i <= j ? 1 : 0));

            assert(above_or_equals(ui, uj) == (ui >= uj ? 1 : 0));
            assert(above_or_equals2(ui, uj) == (ui >= uj ? 1 : 0));
            assert(below_or_equals(ui, uj) == (ui <= uj ? 1 : 0));
            assert(below_or_equals2(ui, uj) == (ui <= uj ? 1 : 0));
        }
    }
    return EXIT_SUCCESS;
}

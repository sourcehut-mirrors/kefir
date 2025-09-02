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
    assert(fabsl(get(0, 0, 3.14L, 2.71L) - 3.14L) < 1.0e-9L);
    assert(fabsl(get(0, 1, 3.14L, 2.71L) - 3.14L) < 1.0e-9L);
    assert(fabsl(get(1, 0, 3.14L, 2.71L) - 2.71L) < 1.0e-9L);
    assert(fabsl(get(1, 1, 3.14L, 2.71L) - 3.14L) < 1.0e-9L);
    return EXIT_SUCCESS;
}

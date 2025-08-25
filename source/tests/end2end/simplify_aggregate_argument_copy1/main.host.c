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
    for (unsigned long i = 0; i < 1024; i++) {
        for (unsigned long j = 0; j < 1024; j++) {
            assert(test2((struct S2){{i, j}}) == i + j);
            assert(test8((struct S8){{i, 0, j, 1, ~j, i | j, 100, i ^ j}}) == i + j + 1 + ~j + (i | j) + 100 + (i ^ j));
        }
    }
    return EXIT_SUCCESS;
}

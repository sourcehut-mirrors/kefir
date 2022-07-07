/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

unsigned long VAL = 0;

int main() {
    for (unsigned int i = 0; i < 1000; i++) {
        for (unsigned int j = 1; j < 1000; j++) {
            assert(udiv(i, j) == i / j);
            assert(umod(i, j) == i % j);
            VAL = i;
            assert(udiv_assn(j) == (i / j));
            assert(VAL == (i / j));
        }

        assert(udiv(~0ull, i + 1) == ~0ull / (i + 1));
        assert(umod(~0ull, i + 1) == ~0ull % (i + 1));

        VAL = ~0ull;
        assert(udiv_assn(i + 1) == (~0ull / (i + 1)));
        assert(VAL == (~0ull / (i + 1)));
    }
    return EXIT_SUCCESS;
}

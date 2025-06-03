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
    assert(a == 1);
    assert(b == 1);
    assert(c == 1);
    assert(d == 1);
    assert(e == 1);
    assert(f == 1);
    assert(g == 1);
    assert(h == 1);
    assert(i == 1);
    assert(j == 1);
    assert(k == 1);

    assert(sizes[0] == 1);
    assert(sizes[1] == 1);
    assert(sizes[2] == 1);
    assert(sizes[3] == 1);
    assert(sizes[4] == 2);
    assert(sizes[5] == 4);
    assert(sizes[6] == 8);
    assert(sizes[7] == 8);
    assert(sizes[8] == 16);
    assert(sizes[9] == 32);
    assert(sizes[10] == 128);

    assert(alignments[0] == 1);
    assert(alignments[1] == 1);
    assert(alignments[2] == 1);
    assert(alignments[3] == 1);
    assert(alignments[4] == 2);
    assert(alignments[5] == 4);
    assert(alignments[6] == 8);
    assert(alignments[7] == 8);
    assert(alignments[8] == 8);
    assert(alignments[9] == 8);
    assert(alignments[10] == 8);
    return EXIT_SUCCESS;
}

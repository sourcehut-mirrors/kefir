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
#include <float.h>
#include "./definitions.h"

int main(void) {
    assert(sizes[0] == 1);
    assert(sizes[1] == 2);
    assert(sizes[2] == 4);
    assert(sizes[3] == 8);
    assert(sizes[4] == 16);
    assert(sizes[5] == 40);

    assert(alignments[0] == 1);
    assert(alignments[1] == 2);
    assert(alignments[2] == 4);
    assert(alignments[3] == 8);
    assert(alignments[4] == 8);
    assert(alignments[5] == 8);
    return EXIT_SUCCESS;
}

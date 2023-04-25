/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
#include <stddef.h>
#include "./definitions.h"

int main(void) {
    assert(Alignments[0] == _Alignof(long));
    assert(Alignments[1] == _Alignof(long));
    assert(Alignments[2] == _Alignof(long));
    assert(Alignments[3] == _Alignof(long));
    assert(Alignments[4] == _Alignof(int));
    assert(Alignments[5] == 1);
    assert(Alignments[6] == 1);
    assert(Alignments[7] == _Alignof(long));
    assert(Alignments[8] == 1);
    assert(Alignments[9] == 1);
    assert(Alignments[10] == _Alignof(int));
    assert(Alignments[11] == _Alignof(long));
    assert(Alignments[12] == _Alignof(long));
    assert(Alignments[13] == 1);
    assert(get_alignment(0) == _Alignof(long));
    assert(get_alignment(1) == _Alignof(long));
    assert(get_alignment(2) == _Alignof(long));
    assert(get_alignment(3) == _Alignof(long));
    assert(get_alignment(4) == _Alignof(int));
    assert(get_alignment(5) == 1);
    assert(get_alignment(6) == 1);
    assert(get_alignment(7) == _Alignof(long));
    assert(get_alignment(8) == 1);
    assert(get_alignment(9) == 1);
    assert(get_alignment(10) == _Alignof(int));
    assert(get_alignment(11) == _Alignof(long));
    assert(get_alignment(12) == _Alignof(long));
    assert(get_alignment(13) == 1);
    return EXIT_SUCCESS;
}

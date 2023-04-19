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
#include <math.h>
#include <stddef.h>
#include "./definitions.h"

int main(void) {
    assert(offsets[0] == offsetof(struct Struct1, a));
    assert(getoffset(0) == offsetof(struct Struct1, a));

    assert(offsets[1] == offsetof(struct Struct1, b));
    assert(getoffset(1) == offsetof(struct Struct1, b));

    assert(offsets[2] == offsetof(struct Struct1, c));
    assert(getoffset(2) == offsetof(struct Struct1, c));

    assert(offsets[3] == offsetof(struct Struct1, d));
    assert(getoffset(3) == offsetof(struct Struct1, d));

    assert(offsets[4] == offsetof(struct Struct1, e));
    assert(getoffset(4) == offsetof(struct Struct1, e));

    assert(offsets[5] == offsetof(struct Struct1, f));
    assert(getoffset(5) == offsetof(struct Struct1, f));

    assert(offsets[6] == offsetof(struct Struct1, g));
    assert(getoffset(6) == offsetof(struct Struct1, g));

    assert(offsets[7] == offsetof(struct Struct1, x));
    assert(getoffset(7) == offsetof(struct Struct1, x));

    assert(offsets[8] == offsetof(union Union1, a));
    assert(getoffset(8) == offsetof(union Union1, a));

    assert(offsets[9] == offsetof(union Union1, b));
    assert(getoffset(9) == offsetof(union Union1, b));

    assert(offsets[10] == offsetof(union Union1, c));
    assert(getoffset(10) == offsetof(union Union1, c));

    assert(offsets[11] == offsetof(struct Struct1, c[2]));
    assert(getoffset(11) == offsetof(struct Struct1, c[2]));

    assert(offsets[12] == offsetof(struct Struct1, e[10]));
    assert(getoffset(12) == offsetof(struct Struct1, e[10]));

    assert(offsets[13] == offsetof(struct Struct1, g.b));
    assert(getoffset(13) == offsetof(struct Struct1, g.b));

    assert(offsets[14] == offsetof(struct Struct1, x[5]));
    assert(getoffset(14) == offsetof(struct Struct1, x[5]));
    return EXIT_SUCCESS;
}

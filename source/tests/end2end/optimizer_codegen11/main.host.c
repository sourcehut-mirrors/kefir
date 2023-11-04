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
#include <string.h>
#include "./definitions.h"

int main(void) {
    struct Struct1 ref;
    memset(&ref, 0, sizeof(struct Struct1));

    struct Struct1 x = get_struct1();
    assert(memcmp(&x, &ref, sizeof(struct Struct1)) == 0);

    struct Struct1 y = {.a = 1, .b = 2, .c = 3, .d = 4};
    assert(memcmp(&y, &ref, sizeof(struct Struct1)) != 0);
    zero_struct1(&y);
    assert(memcmp(&y, &ref, sizeof(struct Struct1)) == 0);

    for (long i = -1000; i < 1000; i++) {
        struct Struct2 ref2 = {i, ~i};
        struct Struct2 z = get_struct2(i);
        assert(memcmp(&z, &ref2, sizeof(struct Struct2)) == 0);
    }
    struct Struct2 ref2;
    memset(&ref2, 0, sizeof(struct Struct2));
    struct Struct2 z;
    zero_struct2(&z);
    assert(memcmp(&z, &ref2, sizeof(struct Struct2)) == 0);

    return EXIT_SUCCESS;
}

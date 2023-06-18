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
    for (char x = -100; x <= 100; x++) {
        for (int y = -500; y <= 500; y++) {
            for (short z = -10; z <= 10; z++) {
                assert(sum1((struct Test1){x, y * 1000, z}) == (x + (y * 1000) + z));
                assert(sum3((struct Test3){x, y, z}) == ((int) x + (long) z));
                assert(sum4(x, y, z, ~x, ~y, ~z, x / 2, y / 2, z / 3) ==
                       ((long) x) + ((long) y) + ((long) z) + ((long) ~x) + ((long) ~y) + ((long) ~z) + ((long) x / 2) +
                           ((long) y / 2) + ((long) z / 3));
                assert(sum5(x, (struct Test4){y, z}, x, (struct Test5){{y, z, x, y}}, z, (struct Test4){x, y}) ==
                       (long) x + (long) y + (long) z + (long) x + (long) y + (long) z + (long) x + (long) y +
                           (long) z + (long) x + (long) y);
            }

            assert(sum2((struct Test2){x, y}) == (-(int) x - (char) y));
        }
    }
    return EXIT_SUCCESS;
}

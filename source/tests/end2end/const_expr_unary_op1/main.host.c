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
    assert(results[0] == -255);
    assert(results[1] == 2);
    assert(results[2] == -65533);
    for (int i = 3; i < 8; i++) {
        assert(results[i] == i + 1);
    }
    for (int i = 8; i < 24; i++) {
        assert(results[i] == (i - 8) % 2);
    }
    assert(results[24] == -1);
    assert(results[25] == -2);
    assert(results[26] == -3);
    assert(results[27] == -4);
    assert(results[28] == 4294967291);
    assert(results[29] == -6);
    assert(results[30] == -7);
    assert(results[31] == -8);
    assert(results[32] == -9);
    assert(results[33] == -10);

    assert(results[34] == -2);
    assert(results[35] == -3);
    assert(results[36] == -4);
    assert(results[37] == -5);
    assert(results[38] == 4294967290);
    assert(results[39] == -7);
    assert(results[40] == -8);
    assert(results[41] == -9);
    assert(results[42] == -10);
    assert(results[43] == -11);

    assert(results[44] == -256);
    assert(results[45] == 1);
    assert(results[46] == -65534);
    assert(results[47] == 3);
    assert(results[48] == 4);
    assert(results[49] == 5);
    assert(results[50] == 6);
    assert(results[51] == 7);
    assert(results[52] == 8);
    assert(results[53] == 9);
    return EXIT_SUCCESS;
}

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

extern int add2(int, int);
extern float fadd3(float, float, float);

float fadd2(float x, float y) {
    return x + y;
}

int main() {
    for (int x = -100; x < 100; x++) {
        for (int y = -100; y < 100; y++) {
            assert(add2(x, y) == x + y);
        }
    }

    for (float x = -10.0; x < 10.0; x += 0.1) {
        for (float y = -10.0; y < 10.0; y += 0.1) {
            for (float z = -10.0; z < 10.0; z += 0.1) {
                assert(fadd3(x, y, z) - (x + y + z) <= 1e-3);
            }
        }
    }

    assert(fnsum == add2);
    assert(fnfsum == fadd2);
    assert(getsum() == add2);
    assert(getfsum() == fadd2);
    return EXIT_SUCCESS;
}

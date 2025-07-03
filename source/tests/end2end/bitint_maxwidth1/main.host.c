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
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    assert(maxwidth == 65535);
    assert(maxsize == 8192);
    assert(maxalign == 8);

    for (unsigned int i = 0; i < maxsize / sizeof(unsigned long); i++) {
        if (i + 1 < (unsigned) maxsize) {
            assert(value.arr[i] == 0);
        } else {
            assert(MASK(value.arr[i], 63) == 0);
        }
    }
    init();
    for (unsigned int i = 0; i < maxsize / sizeof(unsigned long); i++) {
        if (i + 1 < (unsigned) maxsize) {
            assert(value.arr[i] == 0);
        } else {
            assert(MASK(value.arr[i], 63) == 0);
        }
    }

    add(1);
    assert(value.arr[0] == 1);
    for (unsigned int i = 1; i < maxsize / sizeof(unsigned long); i++) {
        if (i + 1 < (unsigned) maxsize) {
            assert(value.arr[i] == 0);
        } else {
            assert(MASK(value.arr[i], 63) == 0);
        }
    }

    mul(-1);
    for (unsigned int i = 0; i < maxsize / sizeof(unsigned long); i++) {
        if (i + 1 < (unsigned) maxsize) {
            assert(value.arr[i] == ~0ull);
        } else {
            assert(MASK(value.arr[i], 63) == (1ull << 63) - 1);
        }
    }

    add(1);
    for (unsigned int i = 0; i < maxsize / sizeof(unsigned long); i++) {
        if (i + 1 < (unsigned) maxsize) {
            assert(value.arr[i] == 0);
        } else {
            assert(MASK(value.arr[i], 63) == 0);
        }
    }
    return EXIT_SUCCESS;
}

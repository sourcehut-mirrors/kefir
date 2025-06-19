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
    assert((test1(0, 0xcafeull) & ((1ull << 43) - 1)) == 0xcafe);
    assert((test1(0, 0xcafeull, 0x0badull) & ((1ull << 43) - 1)) == 0xcafe);
    assert((test1(1, 0xcafeull, 0x0badull) & ((1ull << 43) - 1)) == 0x0bad);
    assert((test1(1, 0xcafeull, 0x0badull, ~0ull) & ((1ull << 43) - 1)) == 0x0bad);
    assert((test1(2, 0xcafeull, 0x0badull, ~0ull) & ((1ull << 43) - 1)) == ((~0ull) & ((1ull << 43) - 1)));
    assert((test1(0, 0xcafeull, 0x0badull, ~0ull, 0ull) & ((1ull << 43) - 1)) == 0xcafe);
    assert((test1(3, 0xcafeull, 0x0badull, ~0ull, 0ull) & ((1ull << 43) - 1)) == 0);
    assert((test1(4, 0xcafeull, 0x0badull, ~0ull, 0ull, 1234ull) & ((1ull << 43) - 1)) == 1234);
    assert((test1(5, 0xcafeull, 0x0badull, ~0ull, 0ull, 1234ull, (unsigned long) -1ll) & ((1ull << 43) - 1)) ==
           (1ull << 43) - 1);
    assert((test1(3, 0xcafeull, 0x0badull, ~0ull, 0ull, 1234ull, (unsigned long) -1ll) & ((1ull << 43) - 1)) == 0);
    assert((test1(6, 0xcafeull, 0x0badull, ~0ull, 0ull, 1234ull, (unsigned long) -1ll, 0x0c0ffeull) &
            ((1ull << 43) - 1)) == 0x0c0ffe);
    assert((test1(7, 0xcafeull, 0x0badull, ~0ull, 0ull, 1234ull, (unsigned long) -1ll, 0x0c0ffeull, 10000ull) &
            ((1ull << 43) - 1)) == 10000);
    assert((test1(8, 0xcafeull, 0x0badull, ~0ull, 0ull, 1234ull, (unsigned long) -1ll, 0x0c0ffeull, 10000ull, 1ull) &
            ((1ull << 43) - 1)) == 1);
    assert((test1(9, 0xcafeull, 0x0badull, ~0ull, 0ull, 1234ull, (unsigned long) -1ll, 0x0c0ffeull, 10000ull, 1ull,
                  0xfefe4e3ull) &
            ((1ull << 43) - 1)) == 0xfefe4e3);
    return EXIT_SUCCESS;
}

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
    assert(test1(0, 0xcafe) == 0xcafe);
    assert(test1(0, 0xcafe, 0x0bad) == 0xcafe);
    assert(test1(1, 0xcafe, 0x0bad) == 0x0bad);
    assert(test1(1, 0xcafe, 0x0bad, ~0ull) == 0x0bad);
    assert(test1(2, 0xcafe, 0x0bad, ~0ull) == ~0ull);
    assert(test1(0, 0xcafe, 0x0bad, ~0ull, 0) == 0xcafe);
    assert(test1(3, 0xcafe, 0x0bad, ~0ull, 0) == 0);
    assert(test1(4, 0xcafe, 0x0bad, ~0ull, 0, 1234) == 1234);
    assert(test1(5, 0xcafe, 0x0bad, ~0ull, 0, 1234, (unsigned long) -1ll) == (unsigned long) -1ll);
    assert(test1(3, 0xcafe, 0x0bad, ~0ull, 0, 1234, (unsigned long) -1ll) == 0);
    assert(test1(6, 0xcafe, 0x0bad, ~0ull, 0, 1234, (unsigned long) -1ll, 0x0c0ffe) == 0x0c0ffe);
    assert(test1(7, 0xcafe, 0x0bad, ~0ull, 0, 1234, (unsigned long) -1ll, 0x0c0ffe, 10000) == 10000);
    assert(test1(8, 0xcafe, 0x0bad, ~0ull, 0, 1234, (unsigned long) -1ll, 0x0c0ffe, 10000, 1) == 1);
    assert(test1(9, 0xcafe, 0x0bad, ~0ull, 0, 1234, (unsigned long) -1ll, 0x0c0ffe, 10000, 1, 0xfefe4e3) == 0xfefe4e3);
    return EXIT_SUCCESS;
}

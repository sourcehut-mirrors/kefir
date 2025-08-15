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
#include <limits.h>
#include "./definitions.h"

#define UMASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))

int main(void) {
    assert(test1(1, (struct S1){{0, 0}}, (struct S1){{0, 0}}));
    assert(test1(1, (struct S1){{~0ull, ~0ull}}, (struct S1){{~0ull, ~0ull}}));
    assert(!test1(1, (struct S1){{~0ull, 0}}, (struct S1){{1, 0}}));
    assert(test1(0, (struct S1){{0, 0}}, (struct S1){{0, 0}}));
    assert(test1(0, (struct S1){{~0ull, ~0ull}}, (struct S1){{~0ull, ~0ull}}));
    assert(!test1(0, (struct S1){{~0ull, 0}}, (struct S1){{1, 0}}));

    assert(test2(1, (struct S1){{1, 0}}, (struct S1){{0, 0}}));
    assert(test2(1, (struct S1){{0, 1}}, (struct S1){{1, 0}}));
    assert(test2(1, (struct S1){{~0ull, ~0ull}}, (struct S1){{0, ~0ull}}));
    assert(!test2(1, (struct S1){{~0ull, 0}}, (struct S1){{0, 1}}));
    assert(test2(0, (struct S1){{1, 0}}, (struct S1){{0, 0}}));
    assert(test2(0, (struct S1){{0, 1}}, (struct S1){{1, 0}}));
    assert(test2(0, (struct S1){{~0ull, ~0ull}}, (struct S1){{0, ~0ull}}));
    assert(!test2(0, (struct S1){{~0ull, 0}}, (struct S1){{0, 1}}));

    assert(!test3(1, (struct S1){{1, 0}}, (struct S1){{0, 0}}));
    assert(!test3(1, (struct S1){{0, 1}}, (struct S1){{1, 0}}));
    assert(!test3(1, (struct S1){{~0ull, ~0ull}}, (struct S1){{0, ~0ull}}));
    assert(test3(1, (struct S1){{~0ull, 0}}, (struct S1){{0, 1}}));
    assert(!test3(0, (struct S1){{1, 0}}, (struct S1){{0, 0}}));
    assert(!test3(0, (struct S1){{0, 1}}, (struct S1){{1, 0}}));
    assert(!test3(0, (struct S1){{~0ull, ~0ull}}, (struct S1){{0, ~0ull}}));
    assert(test3(0, (struct S1){{~0ull, 0}}, (struct S1){{0, 1}}));

    assert(test4(1, (struct S1){{1, 0}}, (struct S1){{0, 0}}));
    assert(test4(1, (struct S1){{~0ull, ~0ull}}, (struct S1){{0, 0}}));
    assert(!test4(1, (struct S1){{~1ull, ~0ull}}, (struct S1){{~0ull, ~0ull}}));
    assert(test4(0, (struct S1){{1, 0}}, (struct S1){{0, 0}}));
    assert(test4(0, (struct S1){{~0ull, ~0ull}}, (struct S1){{0, 0}}));
    assert(!test4(0, (struct S1){{~1ull, ~0ull}}, (struct S1){{~0ull, ~0ull}}));

    assert(!test5(1, (struct S1){{1, 0}}, (struct S1){{0, 0}}));
    assert(!test5(1, (struct S1){{~0ull, ~0ull}}, (struct S1){{0, 0}}));
    assert(test5(1, (struct S1){{~1ull, ~0ull}}, (struct S1){{~0ull, ~0ull}}));
    assert(!test5(0, (struct S1){{1, 0}}, (struct S1){{0, 0}}));
    assert(!test5(0, (struct S1){{~0ull, ~0ull}}, (struct S1){{0, 0}}));
    assert(test5(0, (struct S1){{~1ull, ~0ull}}, (struct S1){{~0ull, ~0ull}}));
    return EXIT_SUCCESS;
}

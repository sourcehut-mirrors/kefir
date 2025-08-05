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
#include <math.h>
#include "./definitions.h"

int main(void) {
    long res;
    assert(!add1(1000, 2000, &res));
    assert(res == 3000);
    assert(!add1(1000, -2000, &res));
    assert(res == -1000);

    assert(!add2(0xffff0000ull, 0x0000ffffull, &res));
    assert(res == 0xffffffffll);
    assert(!add2(-(1ull << 49), -(1ull << 60), &res));
    assert(res == (long) (-(1ull << 49) - (1ull << 60)));

    assert(!add3((struct S2) {{1ull << 62, 0}}, (struct S2) {{(1ull << 62) - 1, 0}}, &res));
    assert(res == ~0ull >> 1);
    assert(add3((struct S2) {{1ull << 63, 0}}, (struct S2) {{(1ull << 63) - 1, 0}}, &res));
    assert(res == -1);
    assert(!add3((struct S2) {{0, 1}}, (struct S2) {{-((1ull << 63) + 1), ~0ull}}, &res));
    assert(res == (1ull << 63) - 1);
    assert(add3((struct S2) {{0, 1}}, (struct S2) {{-(1ull << 63), ~0ull}}, &res));
    assert(res == (long) (1ull << 63));
    assert(add3((struct S2) {{0, 1}}, (struct S2) {{~0ull, ~0ull}}, &res));
    assert(res == -1);
    assert(!add3((struct S2) {{1ull << 63, 0}}, (struct S2) {{~0ull, ~0ull}}, &res));
    assert(res == (1ull << 63) - 1);
    assert(!add3((struct S2) {{~1ull, ~0ull}}, (struct S2) {{1, 0}}, &res));
    assert(res == -1);
    return EXIT_SUCCESS;
}

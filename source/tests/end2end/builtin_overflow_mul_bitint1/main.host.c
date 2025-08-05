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
    assert(!mul1(1000, 2000, &res));
    assert(res == 2000000);
    assert(!mul1(1 << 17, 2, &res));
    assert(res == -(1 << 18));

    assert(!mul2(0xffff0000ull, 0x1000, &res));
    assert(res == 0xffff0000000ll);
    assert(mul2(0xffff0000ull, 0x1000000000, &res));
    assert(res == (long) 0xfff0000000000000ull);

    assert(!mul3((struct S2) {{~0ull >> 3, 0}}, (struct S2) {{4, 0}}, &res));
    assert(res == ~0ull >> 3 << 2);
    assert(mul3((struct S2) {{~0ull >> 3, 0}}, (struct S2) {{8, 0}}, &res));
    assert(res == (long) (~0ull >> 3 << 3));
    assert(mul3((struct S2) {{0x12345, 0}}, (struct S2) {{0, 1}}, &res));
    assert(res == 0);
    assert(mul3((struct S2) {{0x12345, 0}}, (struct S2) {{0, 1}}, &res));
    assert(res == 0);
    assert(!mul3((struct S2) {{0x12345, 0}}, (struct S2) {{16, 0}}, &res));
    assert(res == 0x12345 << 4);
    assert(!mul3((struct S2) {{0x12345, 0}}, (struct S2) {{~0ull, ~0ull}}, &res));
    assert(res == -0x12345);
    assert(!mul3((struct S2) {{~1ull, ~0ull}}, (struct S2) {{~0ull, ~0ull}}, &res));
    assert(res == 2);
    assert(!mul3((struct S2) {{~0ull, ~0ull}}, (struct S2) {{~0ull >> 1, 0ull}}, &res));
    assert(res == (long) ((1ull << 63) + 1));
    assert(mul3((struct S2) {{~0ull, ~0ull}}, (struct S2) {{~0ull, 0ull}}, &res));
    assert(res == 1);
    return EXIT_SUCCESS;
}

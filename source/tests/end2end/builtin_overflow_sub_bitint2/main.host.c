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
    unsigned long res;
    assert(sub1(1000, 2000, &res));
    assert(res == (unsigned long) -1000);
    assert(!sub1(1 << 17, 1 << 30, &res));
    assert(res == -((1 << 17) - 1) + ((1 << 30) - 1));

    assert(!sub2(0xffff0000ull, 0x0000ffffull, &res));
    assert(res == 0xffff0000ull - 0x0000ffffull);
    assert(sub2((1ull << 48), (1ull << 59), &res));
    assert(res == ((1ull << 48) - (1ull << 59)));

    assert(!sub3((struct S2) {{0, 1}}, (struct S2) {{~0ull, 0}}, &res));
    assert(res == 1);
    assert(!sub3((struct S2) {{0, 1 << 3}}, (struct S2) {{0, 1 << 3}}, &res));
    assert(res == 0);
    assert(sub3((struct S2) {{0, 0}}, (struct S2) {{0, 1 << 3}}, &res));
    assert(res == 0ull);
    assert(!sub3((struct S2) {{0, 1}}, (struct S2) {{1, 0}}, &res));
    assert(res == ~0ull);
    assert(!sub3((struct S2) {{~0ull, 0}}, (struct S2) {{1ull << 63, 0}}, &res));
    assert(res == (1ull << 63) - 1);
    assert(sub3((struct S2) {{0, 2}}, (struct S2) {{1, 3}}, &res));
    assert(res == ~0ull);
    assert(sub3((struct S2) {{~0ull, 2}}, (struct S2) {{1, 3}}, &res));
    assert(res == ~0ull - 1);
    assert(!sub3((struct S2) {{0, 0}}, (struct S2) {{~0ull, ~0ull}}, &res));
    assert(res == 1);
    assert(sub3((struct S2) {{~0ull, 0}}, (struct S2) {{~0ull, ~0ull}}, &res));
    assert(res == 0);
    return EXIT_SUCCESS;
}

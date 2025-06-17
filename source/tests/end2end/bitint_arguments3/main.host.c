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

struct S2 a[3];
unsigned long b[3];

int main(void) {
    for (unsigned long i = 0; i < 4096; i++) {
        test1((struct S2) {{~i, i}}, i * 2, (struct S2) {{i >> 1, i + 1}}, i - 1, (struct S2) {{i * 10, i / 10}}, i);

        assert(a[0].arr[0] == ~i);
        assert((a[0].arr[1] & ((1ull << 56) - 1)) == (i & ((1ull << 56) - 1)));

        assert(a[1].arr[0] == (i >> 1));
        assert((a[1].arr[1] & ((1ull << 56) - 1)) == ((i + 1) & ((1ull << 56) - 1)));

        assert(a[2].arr[0] == (i * 10));
        assert((a[2].arr[1] & ((1ull << 56) - 1)) == ((i / 10) & ((1ull << 56) - 1)));

        assert((b[0] & ((1ull << 56) - 1)) == ((i * 2) & ((1ull << 56) - 1)));
        assert((b[1] & ((1ull << 56) - 1)) == ((i - 1) & ((1ull << 56) - 1)));
        assert((b[2] & ((1ull << 56) - 1)) == (i & ((1ull << 56) - 1)));
    }
    return EXIT_SUCCESS;
}

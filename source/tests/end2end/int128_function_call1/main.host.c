/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

extern struct i128 somefn(struct i128 a, struct i128 b, struct i128 c, struct i128 d, struct i128 e, struct i128 f,
                          struct i128 g, struct i128 h, struct i128 i, struct i128 j, struct i128 k, struct i128 l) {
    return (struct i128) {{
        a.arr[0] ^ b.arr[1] ^ c.arr[0] ^ d.arr[0] ^ e.arr[1] ^ f.arr[0] ^ g.arr[0] ^ h.arr[1] ^ i.arr[0] ^ j.arr[0] ^
            k.arr[1] ^ l.arr[0],

        a.arr[1] ^ b.arr[0] ^ c.arr[1] ^ d.arr[1] ^ e.arr[0] ^ f.arr[1] ^ g.arr[1] ^ h.arr[0] ^ i.arr[1] ^ j.arr[1] ^
            k.arr[0] ^ l.arr[1],
    }};
}

int main(void) {
    for (unsigned long x = 0; x < 1000; x++) {
        struct i128 res = test1((struct i128) {{x, 0}});

        assert(res.arr[0] == (x ^ (x + 2) ^ (x + 3) ^ (x + 5) ^ (x + 6) ^ (x + 8) ^ (x + 9) ^ (x + 11)));
        assert(res.arr[1] == ((x + 1) ^ (x + 4) ^ (x + 7) ^ (x + 10)));
    }
    return EXIT_SUCCESS;
}

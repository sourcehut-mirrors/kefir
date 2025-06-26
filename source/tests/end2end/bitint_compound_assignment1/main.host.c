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

struct S3 x = {0}, y = {0};

int main(void) {
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    for (unsigned long i = 0; i < 512; i += 8) {
        for (unsigned long j = 0; j < 64; j += 4) {
            x.arr[0] = i;
            x.arr[1] = 0;
            x.arr[2] = 0;

            struct S3 s3 = add((struct S3) {{j, 0, 0}});
            assert(s3.arr[0] == i + j);
            assert(s3.arr[1] == 0);
            assert(s3.arr[2] == 0);
            assert(x.arr[0] == i + j);
            assert(x.arr[1] == 0);
            assert(x.arr[2] == 0);

            x.arr[0] = i;
            x.arr[1] = 0;
            x.arr[2] = 0;

            s3 = sub((struct S3) {{j, 0, 0}});
            assert(s3.arr[0] == i - j);
            assert(s3.arr[1] == (i >= j ? 0 : ~0ull));
            assert(MASK(s3.arr[2], 22) == MASK(i >= j ? 0 : ~0ull, 22));
            assert(x.arr[0] == i - j);
            assert(x.arr[1] == (i >= j ? 0 : ~0ull));
            assert(MASK(x.arr[2], 22) == MASK(i >= j ? 0 : ~0ull, 22));

            x.arr[0] = i;
            x.arr[1] = 0;
            x.arr[2] = 0;

            s3 = imul((struct S3) {{j, 0, 0}});
            assert(s3.arr[0] == i * j);
            assert(s3.arr[1] == 0);
            assert(MASK(s3.arr[2], 22) == 0);
            assert(x.arr[0] == i * j);
            assert(x.arr[1] == 0);
            assert(MASK(x.arr[2], 22) == 0);

            if (j != 0) {
                x.arr[0] = i;
                x.arr[1] = 0;
                x.arr[2] = 0;

                s3 = idiv((struct S3) {{j, 0, 0}});
                assert(s3.arr[0] == i / j);
                assert(s3.arr[1] == 0);
                assert(MASK(s3.arr[2], 22) == 0);
                assert(x.arr[0] == i / j);
                assert(x.arr[1] == 0);
                assert(MASK(x.arr[2], 22) == 0);

                x.arr[0] = i;
                x.arr[1] = 0;
                x.arr[2] = 0;

                s3 = imod((struct S3) {{j, 0, 0}});
                assert(s3.arr[0] == i % j);
                assert(s3.arr[1] == 0);
                assert(MASK(s3.arr[2], 22) == 0);
                assert(x.arr[0] == i % j);
                assert(x.arr[1] == 0);
                assert(MASK(x.arr[2], 22) == 0);
            }

            x.arr[0] = i;
            x.arr[1] = 0;
            x.arr[2] = ~i;

            s3 = and((struct S3) {{j, 0, ~j}});
            assert(s3.arr[0] == (i & j));
            assert(s3.arr[1] == 0);
            assert(MASK(s3.arr[2], 22) == MASK(~i & ~j, 22));
            assert(x.arr[0] == (i & j));
            assert(x.arr[1] == 0);
            assert(MASK(x.arr[2], 22) == MASK(~i & ~j, 22));

            x.arr[0] = i;
            x.arr[1] = 0;
            x.arr[2] = ~i;

            s3 = or ((struct S3) {{j, 0, ~j}});
            assert(s3.arr[0] == (i | j));
            assert(s3.arr[1] == 0);
            assert(MASK(s3.arr[2], 22) == MASK(~i | ~j, 22));
            assert(x.arr[0] == (i | j));
            assert(x.arr[1] == 0);
            assert(MASK(x.arr[2], 22) == MASK(~i | ~j, 22));

            x.arr[0] = i;
            x.arr[1] = 0;
            x.arr[2] = ~i;

            s3 = xor((struct S3) {{j, 0, ~j}});
            assert(s3.arr[0] == (i ^ j));
            assert(s3.arr[1] == 0);
            assert(MASK(s3.arr[2], 22) == MASK(~i ^ ~j, 22));
            assert(x.arr[0] == (i ^ j));
            assert(x.arr[1] == 0);
            assert(MASK(x.arr[2], 22) == MASK(~i ^ ~j, 22));

            x.arr[0] = i;
            x.arr[1] = 0;
            x.arr[2] = 0;

            s3 = lshift((struct S3) {{j, 0, 0}});
            if (j > 0 && j < sizeof(unsigned long) * CHAR_BIT) {
                assert(s3.arr[0] == (i << j));
                assert(s3.arr[1] == i >> (sizeof(unsigned long) * CHAR_BIT - j));
                assert(MASK(s3.arr[2], 22) == 0);
                assert(x.arr[0] == (i << j));
                assert(x.arr[1] == i >> (sizeof(unsigned long) * CHAR_BIT - j));
                assert(MASK(x.arr[2], 22) == 0);
            }

            x.arr[0] = i;
            x.arr[1] = 0;
            x.arr[2] = 0;

            s3 = arshift((struct S3) {{j, 0, 0}});
            if (j < sizeof(unsigned long) * CHAR_BIT) {
                assert(s3.arr[0] == (i >> j));
                assert(s3.arr[1] == 0);
                assert(MASK(s3.arr[2], 22) == 0);
                assert(x.arr[0] == (i >> j));
                assert(x.arr[1] == 0);
                assert(MASK(x.arr[2], 22) == 0);
            }

            y.arr[0] = i;
            y.arr[1] = 0;
            y.arr[2] = 0;

            s3 = umul((struct S3) {{j, 0, 0}});
            assert(s3.arr[0] == i * j);
            assert(s3.arr[1] == 0);
            assert(MASK(s3.arr[2], 22) == 0);
            assert(y.arr[0] == i * j);
            assert(y.arr[1] == 0);
            assert(MASK(y.arr[2], 22) == 0);

            if (j != 0) {
                y.arr[0] = i;
                y.arr[1] = 0;
                y.arr[2] = 0;

                s3 = udiv((struct S3) {{j, 0, 0}});
                assert(s3.arr[0] == i / j);
                assert(s3.arr[1] == 0);
                assert(MASK(s3.arr[2], 22) == 0);
                assert(y.arr[0] == i / j);
                assert(y.arr[1] == 0);
                assert(MASK(y.arr[2], 22) == 0);

                y.arr[0] = i;
                y.arr[1] = 0;
                y.arr[2] = 0;

                s3 = umod((struct S3) {{j, 0, 0}});
                assert(s3.arr[0] == i % j);
                assert(s3.arr[1] == 0);
                assert(MASK(s3.arr[2], 22) == 0);
                assert(y.arr[0] == i % j);
                assert(y.arr[1] == 0);
                assert(MASK(y.arr[2], 22) == 0);
            }

            y.arr[0] = i;
            y.arr[1] = 0;
            y.arr[2] = 0;

            s3 = rshift((struct S3) {{j, 0, 0}});
            if (j < sizeof(unsigned long) * CHAR_BIT) {
                assert(s3.arr[0] == (i >> j));
                assert(s3.arr[1] == 0);
                assert(MASK(s3.arr[2], 22) == 0);
                assert(y.arr[0] == (i >> j));
                assert(y.arr[1] == 0);
                assert(MASK(y.arr[2], 22) == 0);
            }
        }
    }
    return EXIT_SUCCESS;
}

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
    struct S1 r = test1(1, -1);
    assert(r.arr[0] == ~0ull);
    assert(UMASK(r.arr[1], 56) == UMASK(~0ull, 56));
    r = test1(0, -1);
    assert(r.arr[0] == ~0ull);
    assert(UMASK(r.arr[1], 56) == UMASK(~0ull, 56));

    r = test1(1, 0xcafe);
    assert(r.arr[0] == 0xcafe);
    assert(UMASK(r.arr[1], 56) == 0);
    r = test1(0, 0xcafe);
    assert(r.arr[0] == 0xcafe);
    assert(UMASK(r.arr[1], 56) == 0);

    r = test2(1, ~0ull);
    assert(r.arr[0] == ~0ull);
    assert(UMASK(r.arr[1], 56) == 0);
    r = test2(0, ~0ull);
    assert(r.arr[0] == ~0ull);
    assert(UMASK(r.arr[1], 56) == 0);

    r = test2(1, 0xc0ffe);
    assert(r.arr[0] == 0xc0ffe);
    assert(UMASK(r.arr[1], 56) == 0);
    r = test2(0, 0xc0ffe);
    assert(r.arr[0] == 0xc0ffe);
    assert(UMASK(r.arr[1], 56) == 0);

    assert(test3(1, (struct S1) {{1, 0}}));
    assert(test3(1, (struct S1) {{0, 1}}));
    assert(!test3(1, (struct S1) {{0, 0}}));
    assert(test3(0, (struct S1) {{1, 0}}));
    assert(test3(0, (struct S1) {{0, 1}}));
    assert(!test3(0, (struct S1) {{0, 0}}));

    r = test4(1, (struct S1) {{0, 0}});
    assert(r.arr[0] == 0);
    assert(UMASK(r.arr[1], 56) == 0);
    r = test4(1, (struct S1) {{10, 0}});
    assert(r.arr[0] == (unsigned long) -10);
    assert(UMASK(r.arr[1], 56) == UMASK(~0ull, 56));
    r = test4(1, (struct S1) {{-10, ~0ull}});
    assert(r.arr[0] == 10);
    assert(UMASK(r.arr[1], 56) == 0);
    r = test4(0, (struct S1) {{0, 0}});
    assert(r.arr[0] == 0);
    assert(UMASK(r.arr[1], 56) == 0);
    r = test4(0, (struct S1) {{10, 0}});
    assert(r.arr[0] == (unsigned long) -10);
    assert(UMASK(r.arr[1], 56) == UMASK(~0ull, 56));
    r = test4(0, (struct S1) {{-10, ~0ull}});
    assert(r.arr[0] == 10);
    assert(UMASK(r.arr[1], 56) == 0);

    r = test5(1, (struct S1) {{0, 0}});
    assert(r.arr[0] == ~0ull);
    assert(UMASK(r.arr[1], 56) == UMASK(~0ull, 56));
    r = test5(1, (struct S1) {{~0ull, ~0ull}});
    assert(r.arr[0] == 0);
    assert(UMASK(r.arr[1], 56) == 0);
    r = test5(1, (struct S1) {{0, 0}});
    assert(r.arr[0] == ~0ull);
    assert(UMASK(r.arr[1], 56) == UMASK(~0ull, 56));
    r = test5(1, (struct S1) {{-2, ~0ull}});
    assert(r.arr[0] == 1);
    assert(UMASK(r.arr[1], 56) == 0);
    r = test5(0, (struct S1) {{0, 0}});
    assert(r.arr[0] == ~0ull);
    assert(UMASK(r.arr[1], 56) == UMASK(~0ull, 56));
    r = test5(0, (struct S1) {{~0ull, ~0ull}});
    assert(r.arr[0] == 0);
    assert(UMASK(r.arr[1], 56) == 0);
    r = test5(0, (struct S1) {{0, 0}});
    assert(r.arr[0] == ~0ull);
    assert(UMASK(r.arr[1], 56) == UMASK(~0ull, 56));
    r = test5(0, (struct S1) {{-2, ~0ull}});
    assert(r.arr[0] == 1);
    assert(UMASK(r.arr[1], 56) == 0);

    assert(!test6(1, (struct S1) {{1, 0}}));
    assert(!test6(1, (struct S1) {{0, 1}}));
    assert(test6(1, (struct S1) {{0, 0}}));
    assert(!test6(0, (struct S1) {{1, 0}}));
    assert(!test6(0, (struct S1) {{0, 1}}));
    assert(test6(0, (struct S1) {{0, 0}}));

    assert(test7(1, (struct S1) {{0, 0}}) == 0);
    assert(test7(1, (struct S1) {{1, 0}}) == 1);
    assert(test7(1, (struct S1) {{0, 1}}) == CHAR_BIT * sizeof(long) + 1);
    assert(test7(0, (struct S1) {{0, 0}}) == 0);
    assert(test7(0, (struct S1) {{1, 0}}) == 1);
    assert(test7(0, (struct S1) {{0, 1}}) == CHAR_BIT * sizeof(long) + 1);

    assert(test8(1, (struct S1) {{1, 0}}) == 119);
    assert(test8(1, (struct S1) {{0, 1}}) == 55);
    assert(test8(0, (struct S1) {{1, 0}}) == 119);
    assert(test8(0, (struct S1) {{0, 1}}) == 55);

    assert(test9(1, (struct S1) {{1, 0}}) == 0);
    assert(test9(1, (struct S1) {{0, 1}}) == CHAR_BIT * sizeof(long));
    assert(test9(0, (struct S1) {{1, 0}}) == 0);
    assert(test9(0, (struct S1) {{0, 1}}) == CHAR_BIT * sizeof(long));

    assert(test10(1, (struct S1) {{1, 0}}) == 118);
    assert(test10(1, (struct S1) {{0, 0}}) == 119);
    assert(test10(1, (struct S1) {{~0ull, ~0ull}}) == 119);
    assert(test10(1, (struct S1) {{0, 1}}) == 54);
    assert(test10(0, (struct S1) {{1, 0}}) == 118);
    assert(test10(0, (struct S1) {{0, 0}}) == 119);
    assert(test10(0, (struct S1) {{~0ull, ~0ull}}) == 119);
    assert(test10(0, (struct S1) {{0, 1}}) == 54);

    assert(test11(1, (struct S1) {{0, 0}}) == 0);
    assert(test11(1, (struct S1) {{1, 0}}) == 1);
    assert(test11(1, (struct S1) {{0, 1}}) == 1);
    assert(test11(1, (struct S1) {{1, 1}}) == 2);
    assert(test11(1, (struct S1) {{~0ull, ~0ull}}) == 120);
    assert(test11(0, (struct S1) {{0, 0}}) == 0);
    assert(test11(0, (struct S1) {{1, 0}}) == 1);
    assert(test11(0, (struct S1) {{0, 1}}) == 1);
    assert(test11(0, (struct S1) {{1, 1}}) == 2);
    assert(test11(0, (struct S1) {{~0ull, ~0ull}}) == 120);

    assert(test12(1, (struct S1) {{0, 0}}) == 0);
    assert(test12(1, (struct S1) {{1, 0}}) == 1);
    assert(test12(1, (struct S1) {{0, 1}}) == 1);
    assert(test12(1, (struct S1) {{1, 1}}) == 0);
    assert(test12(1, (struct S1) {{~0ull, ~0ull}}) == 0);
    assert(test12(1, (struct S1) {{~1ull, ~0ull}}) == 1);
    assert(test12(0, (struct S1) {{0, 0}}) == 0);
    assert(test12(0, (struct S1) {{1, 0}}) == 1);
    assert(test12(0, (struct S1) {{0, 1}}) == 1);
    assert(test12(0, (struct S1) {{1, 1}}) == 0);
    assert(test12(0, (struct S1) {{~0ull, ~0ull}}) == 0);
    assert(test12(0, (struct S1) {{~1ull, ~0ull}}) == 1);
    return EXIT_SUCCESS;
}

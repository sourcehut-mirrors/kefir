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
    struct S2 s2 = test1(0, (struct S2) {{1234, 0xbad0c}});
    assert(s2.arr[0] == 1234);
    assert(s2.arr[1] == 0xbad0c);

    s2 = test1(0, (struct S2) {{1234, 0xbad0c}}, (struct S2) {{~0ull, 102030}});
    assert(s2.arr[0] == 1234);
    assert(s2.arr[1] == 0xbad0c);

    s2 = test1(1, (struct S2) {{1234, 0xbad0c}}, (struct S2) {{~0ull, 102030}});
    assert(s2.arr[0] == ~0ull);
    assert(s2.arr[1] == 102030);

    s2 = test1(2, (struct S2) {{1234, 0xbad0c}}, (struct S2) {{~0ull, 102030}}, (struct S2) {{0, 0xfffeee}});
    assert(s2.arr[0] == 0);
    assert(s2.arr[1] == 0xfffeee);

    s2 = test1(3, (struct S2) {{1234, 0xbad0c}}, (struct S2) {{~0ull, 102030}}, (struct S2) {{0, 0xfffeee}},
               (struct S2) {{~0xcafeull, 0xc0ffeull}});
    assert(s2.arr[0] == ~0xcafeull);
    assert(s2.arr[1] == 0xc0ffeull);

    s2 = test1(4, (struct S2) {{1234, 0xbad0c}}, (struct S2) {{~0ull, 102030}}, (struct S2) {{0, 0xfffeee}},
               (struct S2) {{~0xcafeull, 0xc0ffeull}}, (struct S2) {{0xc4ef5a, 0x324fed6}});
    assert(s2.arr[0] == 0xc4ef5a);
    assert(s2.arr[1] == 0x324fed6);

    s2 = test1(5, (struct S2) {{1234, 0xbad0c}}, (struct S2) {{~0ull, 102030}}, (struct S2) {{0, 0xfffeee}},
               (struct S2) {{~0xcafeull, 0xc0ffeull}}, (struct S2) {{0xc4ef5a, 0x324fed6}},
               (struct S2) {{12346421, 0x5ede4fe3}});
    assert(s2.arr[0] == 12346421);
    assert(s2.arr[1] == 0x5ede4fe3);
    return EXIT_SUCCESS;
}

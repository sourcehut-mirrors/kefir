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
#include "./definitions.h"

int main(void) {
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    for (unsigned long i = 0; i < 4096; i++) {
        if (MASK(i, 6) <= 9) {
            assert(test1(i) == 1);
        } else if (MASK(i, 6) >= 10 && MASK(i, 6) <= 19) {
            assert(test1(i) == 2);
        } else if (MASK(i, 6) >= 20 && MASK(i, 6) <= 29) {
            assert(test1(i) == 3);
        } else {
            assert(test1(i) == 4);
        }

        if (MASK(i, 14) <= 99) {
            assert(test2(i) == 1);
        } else if (MASK(i, 14) >= 100 && MASK(i, 14) <= 1999) {
            assert(test2(i) == 2);
        } else if (MASK(i, 14) >= 2000 && MASK(i, 14) <= 2999) {
            assert(test2(i) == 3);
        } else {
            assert(test2(i) == 4);
        }

        if (MASK(i, 29) <= 999) {
            assert(test3(i) == 1);
            assert(test4(i) == 1);
        } else if (MASK(i, 29) >= 1000 && MASK(i, 29) <= 3999) {
            assert(test3(i) == 2);
            assert(test4(i) == 2);
        } else if (MASK(i, 29) >= 4000 && MASK(i, 29) <= 10000) {
            assert(test3(i) == 3);
            assert(test4(i) == 3);
        } else {
            assert(test3(i) == 4);
            assert(test4(i) == 4);
        }
    }

    assert(test5((struct S2) {{0, 0}}) == 1);
    assert(test5((struct S2) {{0x222211114444ffffull, 0}}) == 2);
    assert(test5((struct S2) {{0x222211114444ffffull, 0xaaaa}}) == 3);
    assert(test5((struct S2) {{0x222211114444ffffull, 0xfffe}}) == 3);
    assert(test5((struct S2) {{0x222211114444ffffull, 0xffff1}}) == 4);
    return EXIT_SUCCESS;
}

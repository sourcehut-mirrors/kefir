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

int main(void) {
    assert(test1(0) == 0);
    for (unsigned int i = 0; i < sizeof(char) * CHAR_BIT; i++) {
        assert(test1(1ull << i) == 1);
        if (i > 0) {
            assert(test1((1ull << i) + 1) == 0);
            if (i > 1) {
                assert(test1((1ull << i) + 2) == 0);
                assert(test1((1ull << i) + 3) == 1);
            }
        }
    }
    assert(test1(-4) == 0);
    assert(test1(-3) == 1);
    assert(test1(-2) == 1);
    assert(test1(-1) == 0);

    assert(test2(0) == 0);
    for (unsigned int i = 0; i < sizeof(short) * CHAR_BIT; i++) {
        assert(test2(1ull << i) == 1);
        if (i > 0) {
            assert(test2((1ull << i) + 1) == 0);
            if (i > 1) {
                assert(test2((1ull << i) + 2) == 0);
                assert(test2((1ull << i) + 3) == 1);
            }
        }
    }
    assert(test2(-4) == 0);
    assert(test2(-3) == 1);
    assert(test2(-2) == 1);
    assert(test2(-1) == 0);

    assert(test3(0) == 0);
    for (unsigned int i = 0; i < sizeof(int) * CHAR_BIT; i++) {
        assert(test3(1ull << i) == 1);
        if (i > 0) {
            assert(test3((1ull << i) + 1) == 0);
            if (i > 1) {
                assert(test3((1ull << i) + 2) == 0);
                assert(test3((1ull << i) + 3) == 1);
            }
        }
    }
    assert(test3(-4) == 0);
    assert(test3(-3) == 1);
    assert(test3(-2) == 1);
    assert(test3(-1) == 0);

    assert(test4(0) == 0);
    for (unsigned int i = 0; i < sizeof(long) * CHAR_BIT; i++) {
        assert(test4(1ull << i) == 1);
        if (i > 0) {
            assert(test4((1ull << i) + 1) == 0);
            if (i > 1) {
                assert(test4((1ull << i) + 2) == 0);
                assert(test4((1ull << i) + 3) == 1);
            }
        }
    }
    assert(test4(-4) == 0);
    assert(test4(-3) == 1);
    assert(test4(-2) == 1);
    assert(test4(-1) == 0);
    return EXIT_SUCCESS;
}

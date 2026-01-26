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
#include "./definitions.h"

int main(void) {
    for (int i = -100; i < 100; i++) {
        assert(test1(i) == (i + 1 + 2 + 3 + 4 + 5 - 6 - 8 + 9));
        assert(test2(i) == (i - 1 - 4 - 6 - 5 - 2 + 38 + 1931 + 183 - 4));
        assert(test3(i) == (i + 1 + 4 + 6 + 5 + 2 - 38 - 1931 - 183 + 4));
        assert(testl1(i) == (i + 1 + 2 + 3 + 4 + 5 - 6 - 8 + 9));
        assert(testl2(i) == (i - 1 - 4 - 6 - 5 - 2 + 38 + 1931 + 183 - 4));
        assert(testl3(i) == (i + 1 + 4 + 6 + 5 + 2 - 38 - 1931 - 183 + 4));
        assert(testl4(i) == (i + 0xffffffffll + 1 + 2 + 3));
        assert(testl5(i) == (i - 0x1ffffffffll + 1 + 2 + 3));
    }
    return EXIT_SUCCESS;
}

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
#include <stdint.h>
#include "./definitions.h"

int main(void) {
    for (int i = -100; i < 100; i++) {
        assert(test1(i) == i);
        assert(test2(i) == -i);
        assert(test3(i) == i);
        assert(testl1(i) == i);
        assert(testl2(i) == -i);
        assert(testl3(i) == i);
    }

    assert(test4() == INT32_MIN);
    assert(test5() == INT32_MAX);
    assert(testl4() == INT64_MIN);
    assert(testl5() == INT64_MAX);
    return EXIT_SUCCESS;
}

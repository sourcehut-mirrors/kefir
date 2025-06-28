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
    assert(test1(-1) == 1);
    assert(test1(-10) == 1);
    assert(test1(-17) == 2);

    assert(test2(-1) == 1);
    assert(test2(-10) == 1);
    assert(test2(-17) == 1);
    assert(test2(-255) == 1);
    assert(test2(-300) == 2);

    assert(test3(-1) == 1);
    assert(test3(-10) == 1);
    assert(test3(-17) == 1);
    assert(test3(-255) == 1);
    assert(test3(-300) == 2);

    assert(test4(-1) == 1);
    assert(test4(-10) == 1);
    assert(test4(-17) == 1);
    assert(test4(-255) == 1);
    assert(test4(-300) == 2);

    assert(test5((struct S2) {{~0ull, 0}}) == 1);
    assert(test5((struct S2) {{~0ull - 0xf, 0}}) == 1);
    assert(test5((struct S2) {{~0ull, ~0ull}}) == 2);
    return EXIT_SUCCESS;
}

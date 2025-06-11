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
    assert(get1() == 0x7);
    assert(get2() == 0x7);
    assert(get3() == 0x279);
    assert(get4() == 0x279);
    assert(get5() == 0xfefca6);
    assert(get6() == 0xfefca6);
    assert(get7() == 0x12345678aaull);
    assert(get8() == 0x12345678aaull);
    return EXIT_SUCCESS;
}

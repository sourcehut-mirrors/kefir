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
    assert(get1() == 314);
    assert(get2() == -3141);
    assert(get3() == 31415);

    assert(get4() == 314);
    assert(get5() == -3141);
    assert(get6() == 31415);

    assert(get7() == 314);
    assert(get8() == -3141);
    assert(get9() == 31415);
    return EXIT_SUCCESS;
}

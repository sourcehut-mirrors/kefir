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
    assert(STRUCT1.x.a == 1);
    assert(STRUCT1.x.b == 2);
    assert(STRUCT1.y[0].a == 3);
    assert(STRUCT1.y[0].b == 4);
    assert(STRUCT1.y[1].a == 5);
    assert(STRUCT1.y[1].b == 6);
    assert(STRUCT2 == &STRUCT1);
    return EXIT_SUCCESS;
}

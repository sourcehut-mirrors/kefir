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
#include <math.h>
#include <string.h>
#include "./definitions.h"

int main(void) {
    for (long i = -100; i < 100; i++) {
        struct Struct1 s = {.a = i, .b = i * 10, .x = (unsigned int) i, .y = ~(unsigned int) i};

        assert(geta(&s) == s.a);
        assert(getb(&s) == s.b);
        assert(getx(&s) == s.x);
        assert(gety(&s) == s.y);

        struct Struct1 s2 = {0};
        seta(&s2, i);
        setb(&s2, i * 10);
        setx(&s2, (unsigned int) i);
        sety(&s2, ~(unsigned int) i);

        assert(geta(&s2) == s.a);
        assert(getb(&s2) == s.b);
        assert(getx(&s2) == s.x);
        assert(gety(&s2) == s.y);
    }

    return EXIT_SUCCESS;
}

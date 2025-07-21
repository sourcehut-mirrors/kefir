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
#include <assert.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include "./definitions.h"

int main(void) {
    for (int i = -1024; i < 1024; i++) {
        struct S1 s1 = get(i);
        assert(s1.a == i);
        assert(s1.b == 0);
        assert(s1.c == -i);

        struct S2 s2 = get2(i);
        assert(s2.s1.a == i);
        assert(s2.s1.b == 0);
        assert(s2.s1.c == -i);
    }
    return EXIT_SUCCESS;
}

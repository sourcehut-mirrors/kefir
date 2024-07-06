/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

static int init_integer = 0;

int getinteger(int x) {
    return x + init_integer++;
}

int main(void) {
    for (int a = -1000; a < 1000; a++) {
        struct B local_b = getb(a);
        for (int i = 0; i < 50; i++) {
            assert(local_b.b[i].a == a + init_integer - 1);
        }
    }
    return EXIT_SUCCESS;
}

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

static int sum(int *args, int argc) {
    int res = 0;
    for (int i = 0; i < argc; i++) {
        res += args[i];
    }
    return res;
}

static int product(int *args, int argc) {
    int res = 1;
    for (int i = 0; i < argc; i++) {
        res *= args[i];
    }
    return res;
}

int main(void) {
    assert(process(sum, 0) == 0);
    assert(process(product, 0) == 0);
    assert(process(sum, 0, 1) == 0);
    assert(process(product, 0, 1) == 0);
    assert(process(sum, 1, 1) == 1);
    assert(process(product, 1, 1) == 1);
    assert(process(sum, 2, 1, 2) == 3);
    assert(process(product, 2, 1, 2) == 2);
    assert(process(sum, 3, 1, 2, 3) == 6);
    assert(process(product, 3, 1, 2, 3) == 6);
    assert(process(sum, 4, 1, 2, 3, 4) == 10);
    assert(process(product, 4, 1, 2, 3, 4) == 24);
    assert(process(sum, 5, 1, 2, 3, 4, 5) == 15);
    assert(process(product, 5, 1, 2, 3, 4, 5) == 120);
    return EXIT_SUCCESS;
}

/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
#include <string.h>
#include "./definitions.h"

void *array[10];

void fn1(void) {}

int main(void) {
#ifdef __x86_64__
    memset(array, 0, sizeof(array));

    assert(array[0] == NULL);
    assert(array[1] == NULL);
    assert(array[2] == NULL);
    assert(array[3] == NULL);
    assert(array[4] == NULL);
    init_array();

    void (*fn1_ptr)(void) = fn1;
    assert(array[0] == *(void **) &fn1_ptr);
    assert(array[1] == &array[5]);
    assert(strcmp(array[2], "llo") == 0);
    assert(array[3] == getx());
    union {
        void *ptr;
        double dbl;
    } u1 = {.ptr = array[4]};
    assert(u1.dbl - 4.21 <= 1e-6);
#endif
    return EXIT_SUCCESS;
}

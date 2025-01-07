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

static int dummy_counter = 0;

void dummy_fn(void *ptr1, void *ptr2) {
    static void *ref1 = NULL, *ref2 = NULL;
    dummy_counter++;
    if (ref1 == NULL) {
        ref1 = ptr1;
    }
    if (ref2 == NULL) {
        ref2 = ptr2;
    }
    assert(ptr1 != ptr2);
}

static int test1_callback(int *array, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += array[i];
    }
    return sum;
}

int main(void) {
    for (int i = 1; i <= 1024; i++) {
        assert(test1(i, test1_callback) == i * (i - 1) / 2);
        assert(test2(i, test1_callback) == i * (i - 1) / 2);
        test3(i);
    }
    assert(dummy_counter > 0);

    return EXIT_SUCCESS;
}

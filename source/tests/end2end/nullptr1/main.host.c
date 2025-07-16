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

extern void *conv1(void *);
extern _Bool conv2(void *);
extern void *conv3(void *);
extern void set3(void **);
extern void *get1(void **);
extern void *get2(void *);

int main(void) {
    assert(a == NULL);
    assert(!b);
    assert(c == sizeof(void *));
    assert(d == _Alignof(void *));
    assert(get() == NULL);
    assert(!is(&a));
    assert(is(NULL));
    assert(is2());
    assert(conv1(NULL) == NULL);
    assert(conv1(&a) == NULL);
    assert(!conv2(NULL));
    assert(!conv2(&a));
    assert(conv3(NULL) == NULL);
    assert(conv3(&a) == NULL);

    int arr[19];
    test1(arr);
    assert(arr[0]);
    assert(arr[1]);
    assert(arr[2]);
    assert(!arr[3]);
    assert(!arr[4]);
    assert(arr[5]);
    assert(arr[6]);
    assert(!arr[7]);
    assert(!arr[8]);
    assert(arr[9]);
    assert(arr[10]);
    assert(!arr[11]);
    assert(!arr[12]);
    assert(!arr[13]);
    assert(!arr[14]);
    assert(arr[15]);
    assert(!arr[16]);
    assert(!arr[17]);
    assert(arr[18]);

    void *ptr = &a, *ptr2 = &a;
    _Bool b;
    set1(&ptr);
    assert(ptr == NULL);
    set2(&b);
    assert(!b);
    set3(&ptr2);
    assert(ptr2 == NULL);

    assert(!and1(0));
    assert(!and1(1));
    assert(!and2(0));
    assert(!and2(1));
    assert(!and3());

    assert(!or1(0));
    assert(or1(1));
    assert(!or2(0));
    assert(or2(1));
    assert(!or3());

    void *ptr3 = NULL;
    assert(get1(&ptr3) == NULL);
    assert(get2(NULL) == NULL);
    return EXIT_SUCCESS;
}

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

extern void *a, *b, *c, *f;
extern _Bool d, e, g, h, i, j;

void set1(long);
void set2(long *);
void *get1(void);

int main(void) {
    assert(a == NULL);
    assert(b == NULL);
    assert(c == NULL);
    assert(f == NULL);

    assert(d);
    assert(!e);
    assert(!g);
    assert(!h);
    assert(i);
    assert(j);

    set1(1000);
    assert(a == NULL);
    set2(&(long) {1000});
    assert(a == NULL);

    assert(get1() == NULL);
    a = (void *) &b;
    assert(get1() == NULL);
    return EXIT_SUCCESS;
}

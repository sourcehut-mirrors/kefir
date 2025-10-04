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

#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))

int main(void) {
    int i = 0;
    assert(!a[i++]);
    assert(!a[i++]);
    assert(a[i++]);
    assert(a[i++]);
    assert(!a[i++]);
    assert(!a[i++]);
    assert(!a[i++]);
    assert(!a[i++]);
    assert(a[i++]);
    assert(a[i++]);
    assert(!a[i++]);
    assert(!a[i++]);
    assert(!a[i++]);
    assert(!a[i++]);
    assert(a[i++]);
    assert(a[i++]);
    assert(!a[i++]);
    assert(!a[i++]);

    i = 0;
    assert(!b[i++]);
    assert(!b[i++]);
    assert(!b[i++]);
    assert(!b[i++]);
    assert(b[i++] > 0);
    assert(b[i++] < 0);
    assert(!b[i++]);
    assert(!b[i++]);
    assert(!b[i++]);
    assert(!b[i++]);
    assert(b[i++] > 0);
    assert(b[i++] < 0);
    assert(!b[i++]);
    assert(!b[i++]);
    assert(!b[i++]);
    assert(!b[i++]);
    assert(b[i++] > 0);
    assert(b[i++] < 0);

    i = 0;
    assert(c[i++] == 1);
    assert(c[i++] == 1);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 1);
    assert(c[i++] == 1);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 1);
    assert(c[i++] == 1);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);

    i = 0;
    assert(!d[i++]);
    assert(!d[i++]);
    assert(!d[i++]);
    assert(!d[i++]);
    assert(d[i++] == 1);
    assert(d[i++] == 1);
    assert(!d[i++]);
    assert(!d[i++]);
    assert(!d[i++]);
    assert(!d[i++]);
    assert(d[i++] == 1);
    assert(d[i++] == 1);
    assert(!d[i++]);
    assert(!d[i++]);
    assert(!d[i++]);
    assert(!d[i++]);
    assert(d[i++] == 1);
    assert(d[i++] == 1);
    return EXIT_SUCCESS;
}

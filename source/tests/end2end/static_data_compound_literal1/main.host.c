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
    assert(a == 100);
    assert(b[0] == 200);
    assert(b[1] == 300);
    assert(c.a == -1);
    assert(c.b == -1234);
    assert(c.c == 'c');
    assert(d[0].a == 1000);
    assert(d[0].b == 0);
    assert(d[0].c == 0);
    assert(d[1].a == 0);
    assert(d[1].b == -3);
    assert(d[1].c == 0);
    assert(d[2].a == 0);
    assert(d[2].b == 0);
    assert(d[2].c == 0);
    assert(e.a[0].a == 0);
    assert(e.a[0].b == 0);
    assert(e.a[0].c == 'X');
    assert(e.a[1].a == 100);
    assert(e.a[1].b == 0);
    assert(e.a[1].c == 0);
    assert(e.b == 10004);
    return EXIT_SUCCESS;
}

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
#include <float.h>
#include <math.h>
#include <complex.h>
#include "./definitions.h"

int main(void) {
    assert(a[0] == (unsigned long) 7542558010649117862ll);
    assert(a[1] == (unsigned long) 36);

    assert(b[0] == (unsigned long) 1);
    assert(b[1] == (unsigned long) 0);

    assert(c[0] == (unsigned long) 0);
    assert(c[1] == (unsigned long) 0);

    assert(d[0] == (unsigned long) 627764175505);
    assert(d[1] == (unsigned long) 0);

    assert(e[0] == (unsigned long) -3206162);
    assert(e[1] == (unsigned long) -1);

    assert(f[0] == (unsigned long) -1);
    assert(f[1] == (unsigned long) -1);

    assert(g[0] == (unsigned long) 255);
    assert(g[1] == (unsigned long) 0);

    assert(h[0] == (unsigned long) -1131);
    assert(h[1] == (unsigned long) -1);
    return EXIT_SUCCESS;
}

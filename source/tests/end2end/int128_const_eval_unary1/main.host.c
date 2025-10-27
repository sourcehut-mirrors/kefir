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
    assert(a[0] == ~0xff00ff00ff00ff00ull);
    assert(a[1] == ~0x00ff00ff00ff00ull);
    assert(b[0] == ~0xff00ff00ff00ff00ull);
    assert(b[1] == ~0x00ff00ff00ff00ull);

    assert(c[0] == -0xff00ff00ff00ff00ull);
    assert(c[1] == ~0x00ff00ff00ff00ull);
    assert(d[0] == -0xff00ff00ff00ff00ull);
    assert(d[1] == ~0x00ff00ff00ff00ull);

    assert(e[0] == 0xff00ff00ff00ff00ull);
    assert(e[1] == 0x00ff00ff00ff00ull);
    assert(f[0] == 0xff00ff00ff00ff00ull);
    assert(f[1] == 0x00ff00ff00ff00ull);

    assert(!g);
    assert(h);
    assert(!i);
    assert(j);
    return EXIT_SUCCESS;
}

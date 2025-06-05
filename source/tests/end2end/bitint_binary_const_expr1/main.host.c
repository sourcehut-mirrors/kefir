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
    assert(a[0] == 1157425104503701505ull);
    assert(a[1] == 1157425104503701504ull);

    assert(b[0] == 0);
    assert(b[1] == 0);

    assert(c[0] == 1157425104503701505ull);
    assert(c[1] == (unsigned long) -2301339409316839424ll);

    assert(d[0] == (unsigned long) -1ll);
    assert(d[1] == (unsigned long) -1ll);

    assert(e[0] == 1148417905246863359ll);
    assert(e[1] == 1148417905246863360ll);

    assert(f[0] == (unsigned long) -1ll);
    assert(f[1] == (unsigned long) -1ll);

    assert(g[0] == (unsigned long) 1148417905246863359ll);
    assert(g[1] == (unsigned long) -4503599359983616ll);

    assert(h[0] == 1);
    assert(h[1] == 0);
    return EXIT_SUCCESS;
}

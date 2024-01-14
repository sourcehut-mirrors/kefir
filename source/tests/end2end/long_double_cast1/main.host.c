/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include "./definitions.h"

int main(void) {
    for (int i = -100; i < 100; i++) {
        unsigned int u = (unsigned int) i;
        float f = ((float) i) / 10.0f + 0.02f;
        float d = ((float) i) / 10.743f + 0.021f;
        assert(fabsl(imp_cast(i) - (long double) i) < 1e-6l);
        assert(fabsl(intcld(i) - (long double) i) < 1e-6l);
        assert(fabsl(uintcld(u) - (long double) u) < 1e-6l);
        assert(fabsl(f32cld(f) - (long double) f) < 1e-6l);
        assert(fabsl(f32cld(d) - (long double) d) < 1e-6l);
    }

    for (long double x = -100.0l; x <= 100.0l; x += 0.02l) {
        assert(ldcint(x) == (int) x);
        assert(ldcuint(x + 200.0l) == (unsigned int) (x + 200.0l));
        assert(fabs(ldcf32(x) - (float) x) < 1e-3);
        assert(fabs(ldcf64(x) - (double) x) < 1e-6);
        assert(fabsl(neg(x) + x) < 1e-6l);
        assert(fabsl(inc(x) - (x + 1.0l)) < 1e-6l);
    }
    return EXIT_SUCCESS;
}

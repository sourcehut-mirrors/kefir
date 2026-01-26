/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
    assert(fabs(f32[0] - 3.14f) < 1e-6);
    assert(fabs(f32[1] + 3.14f) < 1e-6);
    assert(fabs(f32[2] - 3.14f) < 1e-6);
    assert(fabs(f32[3] + 3.14f) < 1e-6);
    assert(fabs(f32[4]) < 1e-6 && !signbit(f32[4]));
    assert(fabs(f32[5]) < 1e-6 && signbit(f32[5]));

    assert(fabs(f64[0] - 3.14) < 1e-8);
    assert(fabs(f64[1] + 3.14) < 1e-8);
    assert(fabs(f64[2] - 3.14) < 1e-8);
    assert(fabs(f64[3] + 3.14) < 1e-8);
    assert(fabs(f64[4]) < 1e-8 && !signbit(f64[4]));
    assert(fabs(f64[5]) < 1e-8 && signbit(f64[5]));

    assert(fabsl(f80[0] - 3.14l) < 1e-9l);
    assert(fabsl(f80[1] + 3.14l) < 1e-9l);
    assert(fabsl(f80[2] - 3.14l) < 1e-9l);
    assert(fabsl(f80[3] + 3.14l) < 1e-9l);
    assert(fabsl(f80[4]) < 1e-9l && !signbit(f80[4]));
    assert(fabsl(f80[5]) < 1e-9l && signbit(f80[5]));

    assert(fabs(my_copysignf(3.14f, 1.0f) - 3.14f) < 1e-6);
    assert(fabs(my_copysignf(3.14f, -1.0f) + 3.14f) < 1e-6);
    assert(fabs(my_copysignf(-3.14f, 1.0f) - 3.14f) < 1e-6);
    assert(fabs(my_copysignf(-3.14f, -1.0f) + 3.14f) < 1e-6);
    assert(fabs(my_copysignf(0.0f, 0.0f)) < 1e-6 && !signbit(my_copysignf(0.0f, 0.0f)));
    assert(fabs(my_copysignf(0.0f, -0.0f)) < 1e-6 && signbit(my_copysignf(0.0f, -0.0f)));
    assert(isinf(my_copysignf(-INFINITY, 0.0f)) && !signbit(my_copysignf(-INFINITY, 0.0f)));
    assert(isinf(my_copysignf(INFINITY, -0.0f)) && signbit(my_copysignf(INFINITY, -0.0f)));

    assert(fabs(my_copysign(3.14, 1.0) - 3.14) < 1e-8);
    assert(fabs(my_copysign(3.14, -1.0) + 3.14) < 1e-8);
    assert(fabs(my_copysign(-3.14, 1.0) - 3.14) < 1e-8);
    assert(fabs(my_copysign(-3.14, -1.0) + 3.14) < 1e-8);
    assert(fabs(my_copysign(0.0, 0.0)) < 1e-8 && !signbit(my_copysign(0.0, 0.0)));
    assert(fabs(my_copysign(0.0, -0.0)) < 1e-8 && signbit(my_copysign(0.0, -0.0)));
    assert(isinf(my_copysign(-INFINITY, 0.0)) && !signbit(my_copysign(-INFINITY, 0.0)));
    assert(isinf(my_copysign(INFINITY, -0.0)) && signbit(my_copysign(INFINITY, -0.0)));

    assert(fabsl(my_copysignl(3.14l, 1.0l) - 3.14l) < 1e-9);
    assert(fabsl(my_copysignl(3.14l, -1.0l) + 3.14l) < 1e-9);
    assert(fabsl(my_copysignl(-3.14l, 1.0l) - 3.14l) < 1e-9l);
    assert(fabsl(my_copysignl(-3.14l, -1.0l) + 3.14l) < 1e-9l);
    assert(fabsl(my_copysignl(0.0l, 0.0l)) < 1e-9l && !signbit(my_copysignl(0.0l, 0.0l)));
    assert(fabsl(my_copysignl(0.0l, -0.0l)) < 1e-9l && signbit(my_copysignl(0.0l, -0.0l)));
    return EXIT_SUCCESS;
}

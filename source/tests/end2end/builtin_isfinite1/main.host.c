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
    assert(f32[0]);
    assert(f32[1]);
    assert(f32[2]);
    assert(!f32[3]);
    assert(!f32[4]);
    assert(!f32[5]);

    assert(f64[0]);
    assert(f64[1]);
    assert(f64[2]);
    assert(!f64[3]);
    assert(!f64[4]);
    assert(!f64[5]);

    assert(f80[0]);
    assert(f80[1]);
    assert(f80[2]);
    assert(!f80[3]);
    assert(!f80[4]);
    assert(!f80[5]);

    assert(is_finite_f32((float) 0.0f));
    assert(is_finite_f32((float) 1.0f));
    assert(is_finite_f32((float) -3.14f));
    assert(!is_finite_f32((float) INFINITY));
    assert(!is_finite_f32((float) -INFINITY));
    assert(!is_finite_f32((float) nan("")));

    assert(is_finite_f64((double) 0.0));
    assert(is_finite_f64((double) 1.0));
    assert(is_finite_f64((double) -3.14));
    assert(!is_finite_f64((double) INFINITY));
    assert(!is_finite_f64((double) -INFINITY));
    assert(!is_finite_f64((double) nan("")));

    assert(is_finite_f80((long double) 0.0L));
    assert(is_finite_f80((long double) 1.0L));
    assert(is_finite_f80((long double) -3.14L));
    assert(!is_finite_f80((long double) INFINITY));
    assert(!is_finite_f80((long double) -INFINITY));
    assert(!is_finite_f80((long double) nan("")));
    return EXIT_SUCCESS;
}

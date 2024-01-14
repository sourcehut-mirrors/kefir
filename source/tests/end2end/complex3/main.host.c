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
#include <complex.h>
#include "./definitions.h"

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

int main(void) {
    _Complex float x = 3.14159f + I * 2.1828f;
    _Complex double y = 2.1828 + I * 3.14159;
    _Complex long double z = -12.453 + I * 3371;

    assert(cmpf32(x, x));
    assert(!cmpf32_not(x, x));
    assert(!cmpf32(x, x + 1));
    assert(!cmpf32(x, x + I));
    assert(cmpf32_not(x, x + 1));
    assert(cmpf32_not(x, x + I));

    assert(cmpf64(y, y));
    assert(!cmpf64_not(y, y));
    assert(!cmpf64(y, y + 1));
    assert(!cmpf64(y, y + I));
    assert(cmpf64_not(y, y + 1));
    assert(cmpf64_not(y, y + I));

    assert(cmpld(z, z));
    assert(!cmpld_not(z, z));
    assert(!cmpld(z, z + 1));
    assert(!cmpld(z, z + I));
    assert(cmpld_not(z, z + 1));
    assert(cmpld_not(z, z + I));

    assert(cmpf32_bool(1.0f + I * 0.0f));
    assert(cmpf32_bool(0.0f + I * 1.0f));
    assert(!cmpf32_bool(0.0f + I * 0.0f));
    assert(!cmpf32_bool(-0.0f + I * 0.0f));
    assert(!cmpf32_bool(-0.0f + I * 0.0f));
    assert(!cmpf32_bool(0.0f + I * -0.0f));
    assert(!cmpf32_bool(0.0f + I * +0.0f));

    assert(cmpf64_bool(1.0 + I * 0.0));
    assert(cmpf64_bool(0.0 + I * 1.0));
    assert(!cmpf64_bool(0.0 + I * 0.0f));
    assert(!cmpf64_bool(-0.0 + I * 0.0));
    assert(!cmpf64_bool(-0.0 + I * 0.0));
    assert(!cmpf64_bool(0.0 + I * -0.0));
    assert(!cmpf64_bool(0.0 + I * +0.0));

    assert(cmpld_bool(1.0L + I * 0.0L));
    assert(cmpld_bool(0.0L + I * 1.0L));
    assert(!cmpld_bool(0.0L + I * 0.0L));
    assert(!cmpld_bool(-0.0L + I * 0.0L));
    assert(!cmpld_bool(-0.0L + I * 0.0L));
    assert(!cmpld_bool(0.0L + I * -0.0L));
    assert(!cmpld_bool(0.0L + I * +0.0L));
    return EXIT_SUCCESS;
}

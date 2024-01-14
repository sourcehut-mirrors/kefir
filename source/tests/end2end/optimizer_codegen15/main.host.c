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
#include <string.h>
#include "./definitions.h"

#define EPSILON_F 1e-3f
#define EPSILON_D 1e-6

int main(void) {
    for (float i = -100.0f; i < 100.0f; i += 0.1f) {
        double id = (double) i;
        for (float j = -10.0f; j < 10.0f; j += 0.05f) {
            double jd = (double) j;
            assert(fabs(sumf(i, j) - (i + j)) < EPSILON_F);
            assert(fabs(sumd(id, jd) - (id + jd)) < EPSILON_D);
            assert(fabs(subf(i, j) - (i - j)) < EPSILON_F);
            assert(fabs(subd(id, jd) - (id - jd)) < EPSILON_D);
            assert(fabs(mulf(i, j) - (i * j)) < EPSILON_F);
            assert(fabs(muld(id, jd) - (id * jd)) < EPSILON_D);
            assert(fabs(divf(i, j) - (i / j)) < EPSILON_F);
            assert(fabs(divd(id, jd) - (id / jd)) < EPSILON_D);
        }

        assert(fabs(negf(i) + i) < EPSILON_F);
        assert(fabs(negd((double) i) + (double) i) < EPSILON_D);
    }

    return EXIT_SUCCESS;
}

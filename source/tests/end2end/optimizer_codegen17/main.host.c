/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

float addf(float x, float y) {
    return x + y;
}

double addd(double x, double y) {
    return x + y;
}

float fneg(float a) {
    return -a;
}

double dneg(double a) {
    return -a;
}

int main(void) {
    for (float i = -25.0f; i < 25.0f; i += 0.1f) {
        double id = (double) i;
        for (float j = -5.0f; j < 5.0f; j += 0.01f) {
            assert(fabs(my_hypotf(i, j) - (i * i + j * j)) < EPSILON_F);
            assert(fabs(sumf((union Sumf){.x = i, .y = j}).result - (i + j)) < EPSILON_F);
            assert(fabs(my_hypotf2((union Sumf){.x = i, .y = j}).result - (i * i + j * j)) < EPSILON_F);

            double jd = (double) j;
            assert(fabs(my_hypotd(id, jd) - (id * id + jd * jd)) < EPSILON_D);
            assert(fabs(sumd((union Sumd){.x = id, .y = jd}).result - (id + jd)) < EPSILON_D);
            assert(fabs(my_hypotd2((union Sumd){.x = id, .y = jd}).result - (id * id + jd * jd)) < EPSILON_D);
        }

        assert(fabs(sum10f(-1, i, i * 2, i + 2, i + 3, i * 10, -5 * i, 0.0f, i, i + 100) -
                    (-1 + i + i * 2 + i + 2 + i + 3 + i * 10 - 5 * i + 0.0f + i + i + 100)) < EPSILON_F);
        assert(fabs(sum10d(-1, id, id * 2, id + 2, id + 3, id * 10, -5 * id, 0.0, id, id + 100) -
                    (-1 + id + id * 2 + id + 2 + id + 3 + id * 10 - 5 * id + 0.0 + id + id + 100)) < EPSILON_D);

        float farr[4] = {i, i + 1.0f, 2 * i, 0.0f};
        farr_map(farr, sizeof(farr) / sizeof(farr[0]), fneg);
        assert(fabs(farr[0] + i) < EPSILON_F);
        assert(fabs(farr[1] + i + 1.0f) < EPSILON_F);
        assert(fabs(farr[2] + 2 * i) < EPSILON_F);
        assert(fabs(farr[3]) < EPSILON_F);

        double darr[4] = {id, id + 1.0, 2 * id, 0.0};
        darr_map(darr, sizeof(darr) / sizeof(darr[0]), dneg);
        assert(fabs(darr[0] + id) < EPSILON_D);
        assert(fabs(darr[1] + id + 1.0) < EPSILON_D);
        assert(fabs(darr[2] + 2 * id) < EPSILON_D);
        assert(fabs(darr[3]) < EPSILON_D);
    }

    return EXIT_SUCCESS;
}

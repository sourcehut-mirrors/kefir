/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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
#include "kefir/core/util.h"
#include "kefir/test/unit_test.h"

struct circle_res {
    float length;
    float area;
    float neg_radius;
};

struct circle_res circle(float);

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    for (float i = 0.0f; i < 1000.0f; i += 0.01f) {
        struct circle_res res = circle(i);
        ASSERT(FLOAT_EQUALS(res.length, 2.0f * 3.14159f * i, FLOAT_EPSILON));
        ASSERT(FLOAT_EQUALS(res.area, 3.14159f * ((float) (i * i)) / 2.0f, FLOAT_EPSILON));
        ASSERT(FLOAT_EQUALS(res.neg_radius, -i, FLOAT_EPSILON));
    }
    return EXIT_SUCCESS;
}

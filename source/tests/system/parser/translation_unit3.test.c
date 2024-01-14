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
#include <string.h>
#include "kefir/core/util.h"
#include "kefir/test/unit_test.h"

float progression(float);

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    float base = 0.01f;
    for (int i = -10; i <= 10; i++) {
        for (int j = 0; j < 50; j++) {
            ASSERT(FLOAT_EQUALS(progression(i), i + base, FLOAT_EPSILON));
            base += 0.01f;
        }
    }
    return EXIT_SUCCESS;
}

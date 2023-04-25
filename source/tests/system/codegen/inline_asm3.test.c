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
#include "kefir/core/util.h"
#include "kefir/test/unit_test.h"

#ifdef __x86_64__
double custom_hypot(double, double);
#endif

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);
#ifdef __x86_64__
    for (double x = -100.0; x < 100.0; x += 0.2) {
        for (double y = -100.0; y < 100.0; y += 0.2) {
            ASSERT(DOUBLE_EQUALS(custom_hypot(x, y), x * x + y * y, DOUBLE_EPSILON));
        }
    }
#endif
    return EXIT_SUCCESS;
}

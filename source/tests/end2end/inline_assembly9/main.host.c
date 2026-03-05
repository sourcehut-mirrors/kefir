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
#ifdef __x86_64__
    for (float x = 0.0; x < 100.0; x += 0.1) {
        for (float y = 0.0; y < 100.0; y += 0.1) {
            assert(add(x, y) == ((int) x) + (int) y);
            assert(fabs(addf(x, y) - (x + y)) <= 1.0e-5);
            assert(fabs(addf2(x, y) - (x + y)) <= 1.0e-5);
        }
    }
#endif
    return EXIT_SUCCESS;
}

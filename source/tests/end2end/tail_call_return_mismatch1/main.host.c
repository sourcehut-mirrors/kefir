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
#include <assert.h>
#include <math.h>
#include "./definitions.h"

double conv1(int x) {
    return x;
}

int main(void) {
    for (int x = -4096; x < 4096; x++) {
        union {
            unsigned long lng;
            double dbl;
        } conv = {.lng = test1(x)};
        assert(fabs(conv.dbl - x) < 1e-6);
    }
    return EXIT_SUCCESS;
}

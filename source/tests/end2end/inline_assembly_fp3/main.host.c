/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

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
    for (double x = 0.0; x < 10.0; x += 0.1) {
        double args[] = {x, x + 100.0, x * 3, -x, x + 36.5, x / 1000, x + 1, x - 1, -x / 212, x / 1};
        double expected = 0.0;
        for (unsigned long i = 0; i < sizeof(args) / sizeof(args[0]); i++) {
            expected += args[i];
        }
        assert(fabs(sumall(args) - expected) < 1e-6);
    }
#endif
    return EXIT_SUCCESS;
}

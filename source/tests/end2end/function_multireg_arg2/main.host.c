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
#include <assert.h>
#include <math.h>
#include "./definitions.h"

float call1(struct S1 s) {
    return 1.0f / s.a;
}

float call2(struct S2 s) {
    return s.a[1] / s.a[0];
}

float call3(struct S3 s) {
    return s.a[1] / s.a[0] + s.a[2];
}

int main(void) {
    for (float x = -10.0f; x < 10.0f; x += 0.1f) {
        assert(fabs(call1((struct S1) {x}) - (1.0f / x)) < 1e-3);
        for (float y = -10.0f; y < 10.0f; y += 0.1f) {
            assert(fabs(call2((struct S2) {{x, y}}) - (y / x)) < 1e-3);
            assert(fabs(call3((struct S3) {{x, y, x / y}}) - (y / x + x / y)) < 1e-3);
        }
    }
    return EXIT_SUCCESS;
}

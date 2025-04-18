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

#define DEF(_size)                         \
    struct X##_size call##_size(float x) { \
        struct X##_size s = {0};           \
        for (long i = 0; i < _size; i++) { \
            s.content[i] = x / (i + 1);    \
        }                                  \
        return s;                          \
    }

DEF(1)
DEF(2)
DEF(3)

int main(void) {
    for (float x = -10.0; x < 10.0f; x += 0.1f) {
        struct X1 s1 = call1_proxy(x);
        struct X2 s2 = call2_proxy(x);
        struct X3 s3 = call3_proxy(x);

        assert(fabs(s1.content[0] - x) < 1e-3);
        assert(fabs(s2.content[0] - x) < 1e-3 && fabs(s2.content[1] - x / 2) < 1e-3);
        assert(fabs(s3.content[0] - x) < 1e-3 && fabs(s3.content[1] - x / 2) < 1e-3 &&
               fabs(s3.content[2] - x / 3) < 1e-3);
    }
    return EXIT_SUCCESS;
}

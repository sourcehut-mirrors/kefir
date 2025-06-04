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
#include <float.h>
#include <math.h>
#include <complex.h>
#include "./definitions.h"

int main(void) {
    for (unsigned int i = 0; i < 16; i++) {
        if (i % 2 == 0) {
            assert(a[i] == 0xff);
        } else {
            assert(a[i] == 0x00);
        }

        if (i % 2 == 0) {
            assert(b[i] == 0xff);
        } else {
            assert(b[i] == 0x00);
        }
    }
    assert(b[16] == 0x01);

    assert(c[0] == 0x01);
    assert(d[0] == 0x01);
    for (unsigned int i = 1; i < 16; i++) {
        if (i % 2 != 0 && i < 15) {
            assert(c[i] == 0xff);
        } else {
            assert(c[i] == 0x00);
        }

        if (i % 2 != 0) {
            assert(d[i] == 0xff);
        } else {
            assert(d[i] == 0x00);
        }
    }
    assert(c[15] == 0x00);
    assert(d[16] == 0x01);

    for (unsigned int i = 1; i < 16; i++) {
        if (i % 2 != 0) {
            assert(e[i] == 0xff);
        } else {
            assert(e[i] == 0x00);
        }

        if (i % 2 != 0) {
            assert(f[i] == 0xff);
        } else {
            assert(f[i] == 0x00);
        }
    }
    assert(f[16] == 0x00);

    assert(!g);
    assert(h);
    assert(!i);
    assert(j);
    return EXIT_SUCCESS;
}

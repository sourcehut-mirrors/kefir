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

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6

int main(void) {
    for (unsigned long x = 0; x < 4096; x++) {
        b = (_Bool) x;
        u8 = (unsigned char) x;
        u16 = (unsigned short) x;
        u32 = (unsigned int) x;
        u64 = (unsigned long) x;
        f32 = (float) x;
        f64 = (double) x;

        assert(get_b() == (_Bool) x);
        assert(get_u8() == (unsigned char) x);
        assert(get_u16() == (unsigned short) x);
        assert(get_u32() == (unsigned int) x);
        assert(get_u64() == (unsigned long) x);
        assert(fabs(get_f32() - (float) x) < EPSILON_F);
        assert(fabs(get_f64() - (double) x) < EPSILON_D);
    }
    return EXIT_SUCCESS;
}

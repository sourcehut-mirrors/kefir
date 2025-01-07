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
    for (int i = -50; i <= 50; i++) {
        char r8 = clamp_char((char) i);
        short r16 = clamp_short((short) i);
        int r32 = clamp_int((int) i);
        long r64 = clamp_long((long) i);

        if (i == 0) {
            assert(r8 == 100);
            assert(r16 == 100);
            assert(r32 == 100);
            assert(r64 == 100);
        } else if (i >= -39 && i <= 39) {
            assert(r8 == i / 10);
            assert(r16 == i / 10);
            assert(r32 == i / 10);
            assert(r64 == i / 10);
        } else {
            assert(r8 == -100);
            assert(r16 == -100);
            assert(r32 == -100);
            assert(r64 == -100);
        }
    }
    return EXIT_SUCCESS;
}

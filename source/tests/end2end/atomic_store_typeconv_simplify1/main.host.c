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
    for (unsigned long x = 0; x < 4096; x++) {
        src8 = x;
        src16 = x;
        src32 = x;
        src64 = x;

        copy8();
        copy16();
        copy32();
        copy64();

        assert(dst8 == (unsigned char) x);
        assert(dst16 == (unsigned short) x);
        assert(dst32 == (unsigned int) x);
        assert(dst64 == (unsigned long) x);
    }
    return EXIT_SUCCESS;
}

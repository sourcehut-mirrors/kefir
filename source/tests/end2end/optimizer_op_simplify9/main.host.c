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
#include "./definitions.h"

int main(void) {
    for (long x = -1000; x <= 1000; x++) {
        if (x >= 0) {
            assert(int_shl_const(x) == (x << 1 << 2 << 3));
        }
        assert(int_sar_const(x) == (x >> 2 >> 1 >> 1));
    }

    for (unsigned long x = 0; x <= 2000; x++) {
        assert(int_shr_const(x) == (x >> 1 >> 2 >> 1));
    }
    return EXIT_SUCCESS;
}

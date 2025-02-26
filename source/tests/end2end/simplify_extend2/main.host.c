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
    for (long x = -100; x < 100; x++) {
        assert(uchar_not(x) == (unsigned char) ~(unsigned char) x);
        assert(char_not(x) == (char) ~(char) x);
        assert(ushort_not(x) == (unsigned short) ~(unsigned short) x);
        assert(short_not(x) == (short) ~(short) x);
        assert(uint_not(x) == (unsigned int) ~(unsigned int) x);
        assert(int_not(x) == (int) ~(int) x);
    }
    return EXIT_SUCCESS;
}

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
#include <stdio.h>
#include <assert.h>
#include "./definitions.h"

int main(void) {
    for (int i = -4096; i < 4096; i++) {
        assert(test_char(i) == (char) (((char) i) + ((char) (i + 1)) + ((char) (i * 2))));
        assert(c1 == (char) i);
        assert(c2 == (char) (i + 1));
        assert(c3 == (char) (i * 2));

        assert(test_short(i) == (short) (((short) i) + ((short) (i + 1)) + ((short) (i * 2))));
        assert(s1 == (short) i);
        assert(s2 == (short) (i + 1));
        assert(s3 == (short) (i * 2));

        assert(test_int(i) == (int) (((int) i) + ((int) (i + 1)) + ((int) (i * 2))));
        assert(i1 == (int) i);
        assert(i2 == (int) (i + 1));
        assert(i3 == (int) (i * 2));

        assert(test_long(i) == (long) (((long) i) + ((long) (i + 1)) + ((long) (i * 2))));
        assert(l1 == (long) i);
        assert(l2 == (long) (i + 1));
        assert(l3 == (long) (i * 2));
    }
    return EXIT_SUCCESS;
}

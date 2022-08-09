/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

int main() {
#ifdef __x86_64__
    for (long x = -10; x <= 10; x++) {
        for (long y = -10; y <= 10; y++) {
            for (long z = -10; z <= 10; z++) {
                assert(sum3_one(x, y, z) == (x + y + z + 1));
            }
        }
    }

    for (unsigned int x = 0; x < 0x2ffff; x++) {
        struct S1 s1 = make_s1(x);
        assert(s1.shrt[0] == (x & 0xffff));
        assert(s1.shrt[1] == ((x >> 16) & 0xffff));
        assert(unwrap_s1(s1) == x);
    }
#endif
    return EXIT_SUCCESS;
}

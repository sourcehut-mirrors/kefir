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

int main(void) {
#ifdef __x86_64__
    for (long l1 = 0; l1 < 5000; l1++) {
        for (long l2 = 0; l2 < 100; l2++) {
            long l = l1 + (l2 << 32);
            assert(clear8(l) == (l & ~0xffL));
            assert(clear16(l) == (l & ~0xffffL));
            assert(clear32(l) == 0);
            assert(clear64(l) == 0);

            assert(set8(l) == (l | 0xffL));
            assert(set16(l) == (l | 0xffffL));
            assert(set32(l) == (l | 0xffffffffL));
            assert(set64(l) == (long) 0xffffffffffffffffL);
        }
    }
#endif
    return EXIT_SUCCESS;
}

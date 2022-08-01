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
#include "kefir/core/util.h"
#include "kefir/test/unit_test.h"

struct val {
    long x;
    long y;
};

#ifdef __x86_64__
struct val process(struct val, long);
#endif

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);
#ifdef __x86_64__
    for (long x = 0; x < 500; x++) {
        for (long y = 0; y < 500; y++) {
            struct val r = process((struct val){x, y}, 25);
            ASSERT(r.x == x + 25);
            ASSERT(r.y == y + 26);
        }
    }
#endif
    return EXIT_SUCCESS;
}

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

extern unsigned char a;
extern unsigned char b;
extern unsigned short c;
extern unsigned short d;
extern unsigned int e;
extern unsigned long f;
extern unsigned char g[9];
extern unsigned char h[128];

#define MASK(_value, _width) ((_value) & (((1ull << (_width)) - 1)))

int main(void) {
    assert(MASK(a, 2) == MASK(-1, 2));
    assert(MASK(b, 4) == MASK(-2, 4));
    assert(MASK(c, 10) == MASK(-300, 10));
    assert(MASK(d, 15) == MASK(-500, 15));
    assert(MASK(e, 24) == MASK(-400, 24));
    assert(MASK(f, 33) == MASK(-300, 33));

    assert(g[0] == 56);
    assert(g[1] == 255);
    assert(g[2] == 255);
    assert(g[3] == 255);
    assert(g[4] == 255);
    assert(g[5] == 255);
    assert(g[6] == 255);
    assert(g[7] == 255);
    assert(g[8] == 31);

    assert(h[0] == 156);
    for (unsigned long i = 1; i < 127; i++) {
        assert(h[i] == 255);
    }
    assert(h[127] == 15);
    return EXIT_SUCCESS;
}

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
    assert(a[0] == 0x03);
    assert(a[1] == 0xcc);
    assert(a[2] == 0x03);
    assert(a[3] == 0xcc);
    assert(a[4] == 0x02);
    assert(a[5] == 0xdd);
    assert(a[6] == 0x02);
    assert(a[7] == 0xdd);
    assert(a[8] == 0x01);
    assert(a[9] == 0xee);
    assert(a[10] == 0x01);
    assert(a[11] == 0xee);
    assert(a[12] == 0x00);
    assert(a[13] == 0xff);
    assert(a[14] == 0x00);
    assert(a[15] == 0xff);

    assert(b[0] == 0xf);

    for (unsigned int i = 0; i < 10; i++) {
        assert(c[i] == 0xff);
    }

    for (unsigned int i = 0; i < 10; i++) {
        assert(d[i] == 0xff);
    }

    assert(e[0] == 0x7f);
    for (unsigned int i = 1; i < 10; i++) {
        assert(e[i] == 0);
    }

    for (unsigned int i = 0; i < 10; i++) {
        assert(f[i] == 0xff);
    }

    assert(g[0] == 0x7f);
    for (unsigned int i = 1; i < 10; i++) {
        assert(g[i] == 0);
    }

    assert(h[0] == 0x7fff);
    assert(i[0] == 0x7fff);
    assert(j[0] == 0x7fff);
    assert(k[0] == 0x7fff);

    for (unsigned int i = 0; i < 10; i++) {
        assert(l[i] == 0xff);
    }

    assert(m[0] == 0xff);
    assert(m[1] == 0xff);
    for (unsigned int i = 2; i < 10; i++) {
        assert(m[i] == 0);
    }

    for (unsigned int i = 0; i < 10; i++) {
        assert(n[i] == 0xff);
    }

    assert(o[0] == 0xff);
    assert(o[1] == 0xff);
    for (unsigned int i = 2; i < 10; i++) {
        assert(o[i] == 0);
    }

    assert(p[0] == 511);
    assert(q[0] == 511);
    assert(r[0] == 511);
    assert(s[0] == 511);

    assert(t[0] == 314);
    assert(u[0] == 314);
    assert(v[0] == 1000);
    return EXIT_SUCCESS;
}

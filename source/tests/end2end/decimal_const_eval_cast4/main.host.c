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

#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))

int main(void) {
    int i = 0;
    assert(MASK(a[i++], 25) == MASK(3, 25));
    assert(MASK(a[i++], 25) == MASK(-2, 25));
    assert(MASK(a[i++], 25) == MASK(4818, 25));
    assert(MASK(a[i++], 25) == MASK(-381, 25));
    assert(MASK(a[i++], 25) == MASK(16777215, 25));
    assert(MASK(a[i++], 25) == MASK(-16777216, 25));
    assert(MASK(a[i++], 25) == MASK(0, 25));
    assert(MASK(a[i++], 25) == MASK(16777215, 25));
    assert(MASK(a[i++], 25) == MASK(3, 25));
    assert(MASK(a[i++], 25) == MASK(-2, 25));
    assert(MASK(a[i++], 25) == MASK(4818, 25));
    assert(MASK(a[i++], 25) == MASK(-381, 25));
    assert(MASK(a[i++], 25) == MASK(16777215, 25));
    assert(MASK(a[i++], 25) == MASK(-16777216, 25));
    assert(MASK(a[i++], 25) == MASK(0, 25));
    assert(MASK(a[i++], 25) == MASK(16777215, 25));
    assert(MASK(a[i++], 25) == MASK(3, 25));
    assert(MASK(a[i++], 25) == MASK(-2, 25));
    assert(MASK(a[i++], 25) == MASK(4818, 25));
    assert(MASK(a[i++], 25) == MASK(-381, 25));
    assert(MASK(a[i++], 25) == MASK(16777215, 25));
    assert(MASK(a[i++], 25) == MASK(-16777216, 25));
    assert(MASK(a[i++], 25) == MASK(0, 25));
    assert(MASK(a[i++], 25) == MASK(16777215, 25));

    i = 0;
    assert(MASK(b[i++], 25) == MASK(3, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(4818, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(33554431, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(33554431, 25));
    assert(MASK(b[i++], 25) == MASK(3, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(4818, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(33554431, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(33554431, 25));
    assert(MASK(b[i++], 25) == MASK(3, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(4818, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(33554431, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(0, 25));
    assert(MASK(b[i++], 25) == MASK(33554431, 25));

    i = 0;
    assert(MASK(c[i++], 62) == MASK(3, 62));
    assert(MASK(c[i++], 62) == MASK(-2, 62));
    assert(MASK(c[i++], 62) == MASK(4818                                                                                                                                                                                                                                                            , 62));
    assert(MASK(c[i++], 62) == MASK(-381, 62));
    assert(MASK(c[i++], 62) == MASK(42841840000000000, 62));
    assert(MASK(c[i++], 62) == MASK(-42841840000000000, 62));
    assert(MASK(c[i++], 62) == MASK(0, 62));
    assert(MASK(c[i++], 62) == MASK(2305843009213693951, 62));
    assert(MASK(c[i++], 62) == MASK(3, 62));
    assert(MASK(c[i++], 62) == MASK(-2, 62));
    assert(MASK(c[i++], 62) == MASK(4818, 62));
    assert(MASK(c[i++], 62) == MASK(-381, 62));
    assert(MASK(c[i++], 62) == MASK(42841844000000000, 62));
    assert(MASK(c[i++], 62) == MASK(-42841844000000000, 62));
    assert(MASK(c[i++], 62) == MASK(0, 62));
    assert(MASK(c[i++], 62) == MASK(2305843009213693951, 62));
    assert(MASK(c[i++], 62) == MASK(3, 62));
    assert(MASK(c[i++], 62) == MASK(-2, 62));
    assert(MASK(c[i++], 62) == MASK(4818, 62));
    assert(MASK(c[i++], 62) == MASK(-381, 62));
    assert(MASK(c[i++], 62) == MASK(42841844000000000, 62));
    assert(MASK(c[i++], 62) == MASK(-42841844000000000, 62));
    assert(MASK(c[i++], 62) == MASK(0, 62));
    assert(MASK(c[i++], 62) == MASK(2305843009213693951, 62));

    i = 0;
    assert(MASK(d[i++], 62) == MASK(3, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(4818, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(42841840000000000, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(4611686018427387903, 62));
    assert(MASK(d[i++], 62) == MASK(3, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(4818, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(42841844000000000, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(4611686018427387903, 62));
    assert(MASK(d[i++], 62) == MASK(3, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(4818, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(42841844000000000, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(0, 62));
    assert(MASK(d[i++], 62) == MASK(4611686018427387903, 62));

    i = 0;
    assert(e[i++] == 3);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -2);
    assert(e[i++] == -1);
    assert(MASK(e[i++], 52) == MASK(-1, 52));
    assert(e[i++] == 4818);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -381);
    assert(e[i++] == -1);
    assert(MASK(e[i++], 52) == MASK(-1, 52));
    assert(e[i++] == 42841840000000000);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -42841840000000000);
    assert(e[i++] == -1);
    assert(MASK(e[i++], 52) == MASK(-1, 52));
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -1);
    assert(e[i++] == -1);
    assert(e[i++] == 2251799813685247);
    assert(e[i++] == 3);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -2);
    assert(e[i++] == -1);
    assert(MASK(e[i++], 52) == MASK(-1, 52));
    assert(e[i++] == 4818);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -381);
    assert(e[i++] == -1);
    assert(MASK(e[i++], 52) == MASK(-1, 52));
    assert(e[i++] == 42841844000000000);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -42841844000000000);
    assert(e[i++] == -1);
    assert(MASK(e[i++], 52) == MASK(-1, 52));
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -1);
    assert(e[i++] == -1);
    assert(e[i++] == 2251799813685247);
    assert(e[i++] == 3);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -2);
    assert(e[i++] == -1);
    assert(MASK(e[i++], 52) == MASK(-1, 52));
    assert(e[i++] == 4818);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -381);
    assert(e[i++] == -1);
    assert(MASK(e[i++], 52) == MASK(-1, 52));
    assert(e[i++] == 42841844000000000);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -42841844000000000);
    assert(e[i++] == -1);
    assert(MASK(e[i++], 52) == MASK(-1, 52));
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == 0);
    assert(e[i++] == -1);
    assert(e[i++] == -1);
    assert(e[i++] == 2251799813685247);

    i = 0;
    assert(f[i++] == 3);
    assert(f[i++] == 0);   
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 4818);
    assert(f[i++] == 0);                
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 42841840000000000);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0); 
    assert(f[i++] == 0); 
    assert(f[i++] == 0);               
    assert(f[i++] == 0);                           
    assert(f[i++] == 0);                                   
    assert(f[i++] == -1);                                                                                                                                                                                                                                                              
    assert(f[i++] == -1);
    assert(f[i++] == 4503599627370495);
    assert(f[i++] == 3);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 4818);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 42841844000000000);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == -1);
    assert(f[i++] == -1);
    assert(f[i++] == 4503599627370495);
    assert(f[i++] == 3);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 4818);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 42841844000000000);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == 0);
    assert(f[i++] == -1);
    assert(f[i++] == -1);
    assert(f[i++] == 4503599627370495);

    i = 0;
    assert(g[i++] == 4787730014588108800ll);
    assert(g[i++] == -4772395005541686987ll);
    assert(g[i++] == 131);
    assert(g[i++] == 0);
    assert(MASK(g[i++], 20) == MASK(0, 20));

    i = 0;
    assert(h[i++] == -4787730014588108800ll);
    assert(h[i++] == 4772395005541686986ll);
    assert(h[i++] == -132);
    assert(h[i++] == -1);
    assert(MASK(h[i++], 20) == MASK(-1, 20));

    i = 0;
    assert(j[i++] == 4787730014588108800ll);
    assert(j[i++] == -4772395005541686987ll);
    assert(j[i++] == 131);
    assert(j[i++] == 0);
    assert(MASK(j[i++], 20) == MASK(0, 20));
    return EXIT_SUCCESS;
}

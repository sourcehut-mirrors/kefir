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
    assert(x[0]);
    assert(x[1]);
    assert(x[2]);
    assert(!x[3]);
    assert(!x[4]);
    assert(!x[5]);

    assert(a[0] == 13744);
    assert(a[1] == 852858528);
    assert(a[2] == 2482958292859528ull);

    assert(b[0] == 13744);
    assert(b[1] == 852858528);
    assert(b[2] == 2482958292859528ull);

    assert(c[0] == 1180090941);
    assert(c[1] == 1313560155);
    assert(c[2] == 1494033356);

    assert(d[0] == -1374389535);
    assert(d[1] == 1087035463);
    assert(d[2] == 1346227501);
    assert(d[3] == 1103719115);
    assert(d[4] == -1911327471);
    assert(d[5] == 1126278265);

    assert(e[0] == -1546188227);
    assert(e[1] == -691913360);
    assert(e[2] == 16396);
    assert(e[3] == 0);
    assert(e[4] == -295081433);
    assert(e[5] == -883533183);
    assert(e[6] == 16412);
    assert(e[7] == 0);
    assert(e[8] == -1683452935);
    assert(e[9] == -1927033744);
    assert(e[10] == 16434);
    assert(e[11] == 0);

    assert(f[0] == 1180090941);
    assert(f[1] == 0);
    assert(f[2] == 1313560155);
    assert(f[3] == 0);
    assert(f[4] == 1494033356);
    assert(f[5] == 0);

    assert(g[0] == -1374389535);
    assert(g[1] == 1087035463);
    assert(g[2] == 0);
    assert(g[3] == 0);
    assert(g[4] == 1346227501);
    assert(g[5] == 1103719115);
    assert(g[6] == 0);
    assert(g[7] == 0);
    assert(g[8] == -1911327471);
    assert(g[9] == 1126278265);
    assert(g[10] == 0);
    assert(g[11] == 0);

    assert(h[0] == -1546188227);
    assert(h[1] == -691913360);
    assert(h[2] == 16396);
    assert(h[3] == 0);
    assert(h[4] == 0);
    assert(h[5] == 0);
    assert(h[6] == 0);
    assert(h[7] == 0);
    assert(h[8] == -295081433);
    assert(h[9] == -883533183);
    assert(h[10] == 16412);
    assert(h[11] == 0);
    assert(h[12] == 0);
    assert(h[13] == 0);
    assert(h[14] == 0);
    assert(h[15] == 0);
    assert(h[16] == -1683452935);
    assert(h[17] == -1927033744);
    assert(h[18] == 16434);
    assert(h[19] == 0);
    assert(h[20] == 0);
    assert(h[21] == 0);
    assert(h[22] == 0);
    assert(h[23] == 0);
    return EXIT_SUCCESS;
}

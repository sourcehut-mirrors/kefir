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
    int i = 0;
    assert(a[i++] == 847249408);
    assert(a[i++] == 800066001);
    assert(a[i++] == 847249408 );
    assert(a[i++] == 837230439);
    assert(a[i++] == 847249408);
    assert(a[i++] == 1830907570);
    assert(a[i++] == 2080374784);
    assert(a[i++] == 2013265920);
    assert(a[i++] == 1824615131);
    assert(a[i++] == 911799156);
    assert(a[i++] == 847249408);
    assert(a[i++] == 847249408);
    assert(a[i++] == 847249408);
    assert(a[i++] == 864329818);
    assert(a[i++] == 1835544350);
    assert(a[i++] == 1827723283);

    i = 0;
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == -49648252);
    assert(b[i++] == 803942222);
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == 279627008);
    assert(b[i++] == 813171351);
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == -1116003387);
    assert(b[i++] == 826155717);
    assert(b[i++] == 0);
    assert(b[i++] == 2080374784);
    assert(b[i++] == 0);
    assert(b[i++] == 2013265920);
    assert(b[i++] == 84814991);
    assert(b[i++] == 834666496);
    assert(b[i++] == -671626667);
    assert(b[i++] == 834680068);
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == 0);
    assert(b[i++] == 834666496);
    assert(b[i++] == 303194);
    assert(b[i++] == 838860800);
    assert(b[i++] == -926007936);
    assert(b[i++] == 830453043);
    assert(b[i++] == -1301928502);
    assert(b[i++] == 1815722336);

    i = 0;
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 1733689461);
    assert(c[i++] == -1259864708);
    assert(c[i++] == 1);
    assert(c[i++] == 807010304);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == -1608694571);
    assert(c[i++] == 22930925);
    assert(c[i++] == 314059257);
    assert(c[i++] == 805784885);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 1175828299);
    assert(c[i++] == -438829485);
    assert(c[i++] == -108812803);
    assert(c[i++] == 806593079);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 2080374784);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 2013265920);
    assert(c[i++] == 84814991);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == -671626667);
    assert(c[i++] == 13572);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 303194);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809762816);
    assert(c[i++] == -926007936);
    assert(c[i++] == 2078003);
    assert(c[i++] == 0);
    assert(c[i++] == 809107456);
    assert(c[i++] == 493485632);
    assert(c[i++] == 1971137591);
    assert(c[i++] == 51);
    assert(c[i++] == 807927808);
    return EXIT_SUCCESS;
}

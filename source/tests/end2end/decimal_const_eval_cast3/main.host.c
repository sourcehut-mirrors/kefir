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
    assert(a[i++] == 1259291200);
    assert(a[i++] == 1260291200);
    assert(a[i++] == -886192448);

    i = 0;
    assert(b[i++] == -1530494976);
    assert(b[i++] == 918785406);
    assert(b[i++] == 1233977344);
    assert(b[i++] == 919018237);
    assert(b[i++] == -296517632);
    assert(b[i++] == -1228232581);
    assert(b[i++] == 162517);
    assert(b[i++] == 834666496);
    assert(b[i++] == 3873181);
    assert(b[i++] == 834666496);
    assert(b[i++] == 59380);
    assert(b[i++] == -1312817152);

    i = 0;
    assert(c[i++] == 0);
    assert(c[i++] == 952195850);
    assert(c[i++] == -968585837);
    assert(c[i++] == 812396877);
    assert(c[i++] == 0);
    assert(c[i++] == 1904391700);
    assert(c[i++] == -1937171674);
    assert(c[i++] == 812409499);
    assert(c[i++] == 0);
    assert(c[i++] == -1438379746);
    assert(c[i++] == 1389209785);
    assert(c[i++] == -1335061527);
    assert(c[i++] == 162517);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 3873181);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == 809500672);
    assert(c[i++] == 59380);
    assert(c[i++] == 0);
    assert(c[i++] == 0);
    assert(c[i++] == -1337982976);
    return EXIT_SUCCESS;
}

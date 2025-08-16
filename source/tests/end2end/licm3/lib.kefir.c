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

void test1(int (*ptr)[10], int ilen, int jlen, int base) {
    int i, j;
    i = 0;
outer:
    if (i == ilen)
        return;

    j = 0;
inner:
    if (j == jlen) {
        i++;
        goto outer;
    }

    ptr[i][j] = (base | ilen) - j + (i << jlen);
    j++;
    goto inner;
}

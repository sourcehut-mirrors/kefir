/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

_Bool test1(int x, int y, int z, int w) {
    return x && y || z && w;
}

int test2(int x) {
    void *jump1 = &&label1;
    void *jump2 = &&label2;

    if (x && 1) {
        goto *jump1;
    } else {
        goto *jump2;
    }
label1:
    return 1;
label2:
    return 2;
}

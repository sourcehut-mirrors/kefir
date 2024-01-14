/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

void none(void) {
    2 + 3 * 4;
}

int test1(void) {
    if (1 & 3) {
        return 2 + 3;
    } else if (6 ^ 5 << 1) {
        return 7 - 1 % 4;
    }

    return 10u % 5 / 100ul;
}

long test2(void) {
    while (1 | 4 - 5) {
        return -1 ^ ~1;
    }
}

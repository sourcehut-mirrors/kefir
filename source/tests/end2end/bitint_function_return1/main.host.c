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

struct S4 get1(int x) {
    unsigned long sign = x < 0 ? ~0ull : 0ull;
    return (struct S4) {{x, sign, sign, sign}};
}

int main(void) {
    for (int i = -100; i < 100; i++) {
        struct S4 s4 = test1(i);
        assert(s4.arr[0] == ~(unsigned long) i);
        unsigned long sign = i >= 0 ? ~0ull : 0ull;
        assert(s4.arr[1] == sign);
        assert(s4.arr[2] == sign);
        assert(s4.arr[3] == sign);
    }
    return EXIT_SUCCESS;
}

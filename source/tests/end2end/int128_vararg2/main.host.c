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
#include <math.h>
#include "./definitions.h"

int main(void) {
    for (int i = -1024; i < 1024; i++) {
        struct i128 res = test(i);
        assert(res.arr[0] == (unsigned long) i);
        assert(res.arr[1] == (unsigned long) (i >= 0 ? 0 : -1));

        res = test(i * 2);
        assert(res.arr[0] == (unsigned long) i * 2);
        assert(res.arr[1] == (unsigned long) (i >= 0 ? 0 : -1));
    }
    return EXIT_SUCCESS;
}

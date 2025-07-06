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
#include <math.h>
#include "./definitions.h"

int main(void) {
    for (int i = -100; i < 100; i++) {
        enum enum1 x = get(i);
        switch (i) {
            case 0:
                assert(x == CONST1);
                break;

            case 1:
                assert(x == CONST2);
                break;

            case 2:
                assert(x == CONST3);
                break;

            case 3:
                assert(x == CONST4);
                break;

            default:
                assert(x == CONST5);
                break;
        }
    }
    return EXIT_SUCCESS;
}

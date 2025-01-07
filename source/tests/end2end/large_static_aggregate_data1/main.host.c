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
    for (int i = 0; i < 514; i++) {
        switch (i) {
            case 280:
                assert(somelargearray[i] == -1);
                break;

            case 376:
                assert(somelargearray[i] == 100);
                break;

            case 513:
                assert(somelargearray[i] == 512);
                break;

            default:
                assert(somelargearray[i] == 0);
                break;
        }
    }

    for (int i = 0; i < 512; i++) {
        assert(test1.arr1[i] == 0);
        if (i == 300) {
            assert(test1.arr2[i] == -123);
        } else {
            assert(test1.arr2[i] == 0);
        }
    }
    return EXIT_SUCCESS;
}

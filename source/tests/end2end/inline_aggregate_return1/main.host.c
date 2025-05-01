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
#include <assert.h>
#include "./definitions.h"

int main(void) {
    for (int i = 0; i < 16; i++) {
        struct S1 s = gets1(i);
        for (int j = 0; j < 16; j++) {
            if (j == i) {
                assert(s.arr[j] == i);
            } else {
                assert(s.arr[j] == 0);
            }
        }
    }
    return EXIT_SUCCESS;
}

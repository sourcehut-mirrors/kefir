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

static char arr[100];
const void *ptr1 = &arr[0];

int main(void) {
    for (int i = 1; i < 100; i++) {
        add(100);
        sub(99);
        assert(ptr1 == &arr[i]);
    }

    for (int i = 99; i > 0; i--) {
        assert(ptr1 == &arr[i]);
        add(99);
        sub(100);
    }
    return EXIT_SUCCESS;
}

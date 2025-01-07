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
    for (float x = -100.0f; x < 100.0f; x += 0.5f) {
        for (float y = -5.0f; y < 5.0f; y += 0.1f) {
            assert(greater_or_equalsf(x, y) == (x >= y ? 1 : 0));
            assert(less_or_equalsf(x, y) == (x <= y ? 1 : 0));
        }
    }
    return EXIT_SUCCESS;
}

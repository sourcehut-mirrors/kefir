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
    for (float i = -10.0f; i < 9.0f; i += 0.05f) {
        assert(compare1(i) == COMPARE1(i));
        assert(compare2(i) == COMPARE2(i));
    }
    for (double i = -10.0; i < 9.0; i += 0.05) {
        assert(compare3(i) == COMPARE3(i));
        assert(compare4(i) == COMPARE4(i));
    }
    return EXIT_SUCCESS;
}

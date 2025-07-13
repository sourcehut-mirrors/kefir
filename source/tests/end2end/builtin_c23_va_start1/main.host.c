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
    assert(sum(0) == 0);
    assert(sum(0, 1l) == 0);
    assert(sum(1, 1l) == 1);
    assert(sum(1, 1l, 10l) == 1);
    assert(sum(2, 1l, 10l) == 11);
    assert(sum(7, 1l, 2l, 3l, 4l, 5l, 6l, 7l) == 28);
    return EXIT_SUCCESS;
}

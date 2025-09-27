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
    assert(x == 823317928ll);
    assert(y[0] == 767246825ll);
    assert(y[1] == 819142904ll);
    assert(z[0] == 889893065ll);
    assert(z[1] == -405455524ll);
    assert(z[2] == -558933437ll);
    assert(z[3] == 808212005ll);
    return EXIT_SUCCESS;
}

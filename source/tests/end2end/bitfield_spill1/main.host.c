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
    assert(x.f0 == -1);
    assert(x.f1 == -104);
    assert(x.f2 == 1343);
    assert(x.f3 == 6);
    assert(x.f4 == 8);
    assert(x.f5 == 41643);
    assert(x.f6 == 0);
    assert(x.f7 == 402);
    assert(x.f8 == 98);
    return EXIT_SUCCESS;
}

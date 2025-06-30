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
#include <float.h>
#include "./definitions.h"

int main(void) {
#define MASK(_x, _y) ((_x) & ((1ull << (_y)) - 1))
    assert(MASK(get1(), 60) == 314);
    assert(MASK(get2(), 60) == 314);
    assert(MASK(get3(), 60) == 2718);
    assert(MASK(get4(), 60) == 2718);
    assert(MASK(get5(), 60) == 271828);
    assert(MASK(get6(), 60) == 271828);

    assert(MASK(get7(), 60) == MASK((unsigned long) -314, 60));
    assert(MASK(get8(), 60) == MASK((unsigned long) -314, 60));
    assert(MASK(get9(), 60) == MASK((unsigned long) -2718, 60));
    assert(MASK(get10(), 60) == MASK((unsigned long) -2718, 60));
    assert(MASK(get11(), 60) == MASK((unsigned long) -271828, 60));
    assert(MASK(get12(), 60) == MASK((unsigned long) -271828, 60));
    return EXIT_SUCCESS;
}

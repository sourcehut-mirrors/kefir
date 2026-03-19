/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
#include <stddef.h>
#include "./definitions.h"

int main(void) {
    assert(s2.f0 == 1);
    assert(s2.f1 == 2);
    assert(s2.f2 == 3);
    assert(s2.f3 == 4);
    assert(s2.f4 == 5);
    assert(s2.f5.f0 == 6);
    assert(s2.f5.f1 == 7);
    assert(s2.f5.f2 == 8);
    assert(s2.f5.f3 == 9);
    assert(s2.f5.f4 == 10);
    assert(s2.f6 == 11);
    assert(s2.f7 == 12);
    assert(s2.f8 == 13);
    return EXIT_SUCCESS;
}

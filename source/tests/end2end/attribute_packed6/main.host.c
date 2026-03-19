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
#if defined(__GNUC__) || defined(__clang__) || defined(__KEFIRCC__)
    assert(s2.f0 == 1);
    assert(s2.f1.f0 == 2);
    assert(s2.f1.f1 == 3);
    assert(s2.f1.f2 == 4);
    assert(s2.f1.f3 == 5);
    assert(s2.f1.f4 == 6);
    assert(s2.f1.f5 == 7);
    assert(s2.f1.f6 == 8);
    assert(s2.f1.f7 == 9);
    assert(s2.f2 == 10);
    assert(s2.f3 == 11);
    assert(s2.f4 == 12);
    assert(s2.f5 == 13);
    assert(s2.f6 == 14);
    assert(s2.f7 == 15);
    assert(s2.f8 == 16);
    assert(s2.f9.f0 == 17);
    assert(s2.f9.f1 == 18);
    assert(s2.f9.f2 == 19);
    assert(s2.f9.f3 == 20);
    assert(s2.f9.f4 == 21);
    assert(s2.f9.f5 == 22);
    assert(s2.f9.f6 == 23);
    assert(s2.f9.f7 == 24);
#endif
    return EXIT_SUCCESS;
}

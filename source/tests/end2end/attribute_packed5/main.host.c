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
    assert(s2.f1 == 2);
    assert(s2.f2.f0 == 3);
    assert(s2.f2.f1 == 4);
    assert(s2.f2.f2 == 5);
    assert(s2.f2.f3 == 6);
    assert(s2.f3 == 7);
    assert(s2.f4 == 8);
#endif
    return EXIT_SUCCESS;
}

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
#include <math.h>
#include <complex.h>
#include "./definitions.h"

struct S1 s1 = {{-1, 2, 3, 4}, 5};

int main(void) {
    struct S1 r1 = get();
    assert(r1.f0.f0 == s1.f0.f0);
    assert(r1.f0.f1 == s1.f0.f1);
    assert(r1.f0.f2 == s1.f0.f2);
    assert(r1.f0.f3 == s1.f0.f3);
    assert(r1.f1 == s1.f1);
    return EXIT_SUCCESS;
}

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

#define MAIN
#include "./definitions.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

struct S1 value = {65535UL,{0,-394,-51,3}};

int main(void) {
    struct S1 res = gets1();
    assert(res.f0 == value.f0);
    assert(res.f1.f0 == value.f1.f0);
    assert(res.f1.f1 == value.f1.f1);
    assert(res.f1.f2 == value.f1.f2);
    assert(res.f1.f3 == value.f1.f3);
    return EXIT_SUCCESS;
}

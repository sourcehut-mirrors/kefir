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
#include "./definitions.h"

int main(void) {
    _Atomic char i8 = 0;
    _Atomic short i16 = 0;
    _Atomic int i32 = 0;
    _Atomic long i64 = 0;

    assert(test1(&i8) == 0);
    assert(i8 == 1);
    assert(test1(&i8) == 1);
    assert(i8 == 1);

    assert(test2(&i16) == 0);
    assert(i16 == 1);
    assert(test2(&i16) == 1);
    assert(i16 == 1);

    assert(test3(&i32) == 0);
    assert(i32 == 1);
    assert(test3(&i32) == 1);
    assert(i32 == 1);

    assert(test4(&i64) == 0);
    assert(i64 == 1);
    assert(test4(&i64) == 1);
    assert(i64 == 1);
    return EXIT_SUCCESS;
}

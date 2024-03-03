/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include <stdbool.h>
#include "./definitions.h"

static int fn_id = 0;

void fn1(void) {
    assert(fn_id == 0);
    fn_id = 1;
}

void fn2(void) {
    assert(fn_id == 0);
    fn_id = 2;
}

void fn3(void) {
    assert(fn_id == 0);
    fn_id = 3;
}

void fn4(void) {
    assert(fn_id == 0);
    fn_id = 4;
}

void fn5(void) {
    assert(fn_id == 0);
    fn_id = 5;
}

int main(void) {
    for (int i = 1; i <= 5; i++) {
        fn_id = 0;
        test(i);
        assert(fn_id == i);
    }
    return EXIT_SUCCESS;
}

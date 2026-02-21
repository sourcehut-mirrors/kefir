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

int global1 = 0xcafebad0;

int fn(int x) {
    return ~x;
}

int main(void) {
    for (int i = -4096; i < 4096; i++) {
        assert(test1(&(int) {i}) == i + i);
        assert(test2(&(int) {i}) == i + i);
        assert(test3(&(int) {i}) == i + i);
        assert(test4(&(int) {i}) == i + i);
        assert(test5(&(int) {i}) == i + i);
        assert(test6(&(int) {i}) == i + i);
        assert(test7(&(int) {i}) == i + i);
    }
    return EXIT_SUCCESS;
}

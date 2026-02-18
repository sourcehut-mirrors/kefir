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
        assert(test2(&(volatile int) {i}) == i + i);
        assert(test3((_Atomic int *) &(int) {i}) == i + i);
        assert(test4(i) == fn(i) * fn(-i) + fn(i));
        assert(test5(i) == global1 * fn(i) + global1);
        assert(test6(i) == global1 * i + global1);
        assert(test7((_Atomic int *) &(int) {i}) == global1 + i + global1);
        assert(test8((_Atomic int *) &(int) {i}) == fn(1000) + i + fn(1000));
        assert(test9(&(int) {i}) == (i ? i + i : -1));
        assert(test10(&(int) {i}) == i * 11);
        assert(test11(&(volatile int) {i}) == i * 11);
        assert(test12(&(int) {i}, (_Atomic int *) &(int) {0}) == i * 11);
    }
    return EXIT_SUCCESS;
}

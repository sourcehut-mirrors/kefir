/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

volatile double X;

void test1(volatile int x, volatile double y, char arr[volatile]) {}

void test2(int x, void **volatile const y, void *z) {}

void test3(int x, int y, int z, int a, int b, int c, volatile int f) {}

void test4() {
    const volatile long a = 1;
    void *volatile b = 0;
    b = a;
}

void test5(volatile double x, double y) {
    X = x + y;
}

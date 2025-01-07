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

void test1(void);
void test2();
int test3(int, long, _Complex long double);
long test4(short, int, ...);
int test5(X, Y, Z);
float test6(int a, long, double b, _Complex float c, short, ...);

void temp() {
    test1();
    test2(1, 2, 3);
    test3(3, 4, 5);
    test4(1, 2, 3, 4, 5);
    test5(1, 2, 3);
    test6(1, 2, 3, 4, 5, 6, 7, 8);

    extern long test7(int);
    test7(100);
}
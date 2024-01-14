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

void test1(long x) {
    while (x)
        ;
}

void test2(long x) {
    while (x > 0)
        ;
}

void test3(long x) {
    while (x >= 0)
        ;
}

void test4(long x) {
    while (x < 0)
        ;
}

void test5(long x) {
    while (x <= 0)
        ;
}

void test6(unsigned long x) {
    while (x > 0)
        ;
}

void test7(unsigned long x) {
    while (x >= 0)
        ;
}

void test8(unsigned long x) {
    while (x < 0)
        ;
}

void test9(unsigned long x) {
    while (x <= 0)
        ;
}

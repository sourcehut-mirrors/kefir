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

void test(int);

void __attribute__((always_inline)) test1() {
    test(100);
}

void __attribute__((__always_inline__)) test2() {
    test(101);
}

void __attribute__((noinline)) test3() {
    test(200);
}

void __attribute__((__noinline__)) test4() {
    test(201);
}

void __attribute__((noipa)) test5() {
    test(300);
}

void __attribute__((__noipa__)) test6() {
    test(301);
}

void main() {
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
}

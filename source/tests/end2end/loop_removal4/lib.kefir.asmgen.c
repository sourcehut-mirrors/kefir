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

int fn1(void);

void test1(void) {
    for (; 1;) {}
}

void test2(void) {
    for (; 0;) {}
}

void test3(void) {
    for (; fn1();) {}
}

void test4(void) {
    for (int i = 1; i; i = 0) {
        fn1();
    }
}

void test5(void) {
    for (int i = 0; i; i = 1) {
        fn1();
    }
}

void test6(void) {
    for (int i = 1; !i; i = 0) {
        fn1();
    }
}

void test7(void) {
    for (int i = 0; !i; i = 1) {
        fn1();
    }
}

void test8(void) {
    for (int i = 0; !i; i = 0) {
        fn1();
    }
}

void test9(void) {
    for (int i = 0; ~i; i = 0) {
        fn1();
    }
}

void test10(void) {
    for (int i = ~0u; ~i; i = 0) {
        fn1();
    }
}

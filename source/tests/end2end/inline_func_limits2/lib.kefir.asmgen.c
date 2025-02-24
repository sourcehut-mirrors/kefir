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

extern void test0(void);

inline void test1() {
    test0();
}

inline void test2() {
    test1();
}

inline void test3() {
    test2();
}

inline void test4() {
    test3();
}

inline void test5() {
    test4();
}

inline void test6() {
    test5();
}

inline void test7() {
    test6();
}

void main() {
    test5();
}

void main1() {
    test6();
}

void main2() {
    test7();
}

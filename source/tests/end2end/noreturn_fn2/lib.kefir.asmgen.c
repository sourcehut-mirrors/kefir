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

void do_exit();

_Noreturn void test1(void) {
    do_exit();
}

inline _Noreturn void test2(void) {
    do_exit();
}

[[noreturn]] void test3(void) {
    do_exit();
}

[[__noreturn__]] void test4(void) {
    do_exit();
}

[[_Noreturn]] void test5(void) {
    do_exit();
}

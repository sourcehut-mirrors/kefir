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

struct S0 {
    char arr[0];
};

void fn(struct S0 *, struct S0 *);

void test1() {
    struct S0 p1[] = {};
    struct S0 p2[] = {};
}

void test2() {
    struct S0 p1[] = {};
    struct S0 p2[] = {};
    fn(p1, p2);
}

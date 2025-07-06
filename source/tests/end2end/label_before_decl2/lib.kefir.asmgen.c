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

void test1(void) {
label1:
}

void test2(void) {
    {
    label1:
    }
    {
    label2:
    }
{label3 : } label4:
}

void test3(void) {
label1:
    int x;
label2:
}

void test4(void) {
label1:
    int x = 1;
label2:
}

void test5(void) {
label1:
    int x = 1, y = 2, z, w = 3;
label2:
}

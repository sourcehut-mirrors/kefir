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

void fn1(_BitInt(6));
void fn2(_BitInt(14));
void fn3(_BitInt(29));
void fn4(_BitInt(62));
void fn5(_BitInt(122));
void fn6(_BitInt(360));

void test1(void) {
    fn1(23wb);
}

void test2(void) {
    fn2(-4096wb);
}

void test3(void) {
    fn3(0xcafe22wb);
}

void test4(void) {
    fn4(0xbad0c0ffewb);
}

void test5(void) {
    fn5(0xeeeeaaaa00002222ccccwb);
}

void test6(void) {
    fn6(0xeeeeaaaa00002222cccc4444222233330000ccccddddaaaaeeeebbbbwb);
}

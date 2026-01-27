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

#include "./definitions.h"

int test1(int x) {
    return ~(~x);
}

int test2(int x) {
    return ~(~(~(~(~x))));
}

int test3(int x) {
    return ~(~(~(~(~(~x)))));
}

int test4() {
    return ~0x80000000u;
}

int test5() {
    return ~0x80000001u;
}

long testl1(long x) {
    return ~(~x);
}

long testl2(long x) {
    return ~(~(~(~(~x))));
}

long testl3(long x) {
    return ~(~(~(~(~(~x)))));
}

long testl4() {
    return ~0x8000000000000000ull;
}

long testl5() {
    return ~0x8000000000000001ull;
}

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
    return x + 1 + 2 + 3 + 4 + 5 - 6 - 8 + 9;
}

int test2(int x) {
    return x - 1 - 4 - 6 - 5 - 2 + 38 + 1931 + 183 - 4;
}

int test3(int x) {
    return x + 1 + 4 + 6 + 5 + 2 - 38 - 1931 - 183 + 4;
}

long testl1(long x) {
    return x + 1l + 2l + 3l + 4l + 5l - 6l - 8l + 9l;
}

long testl2(long x) {
    return x - 1l - 4l - 6l - 5l - 2l + 38l + 1931l + 183l - 4l;
}

long testl3(long x) {
    return x + 1l + 4l + 6l + 5l + 2l - 38l - 1931l - 183l + 4l;
}

long testl4(long x) {
    return x + 0xffffffffll + 1 + 2 + 3;
}

long testl5(long x) {
    return x - 0x1ffffffffll + 1 + 2 + 3;
}

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

#include "./definitions.h"

long test1(long x) {
    return (x + 1) + 2;
}

long test2(long x) {
    return 4 + (x + 3);
}

long test3(long x) {
    return (x - 5) + 10;
}

long test4(long x) {
    return (5 - x) + 13;
}

long test5(long x) {
    return 20 + (x - 13);
}

long test6(long x) {
    return 30 + (12 - x);
}

long test7(long x) {
    return (x + 49) - 150;
}

long test8(long x) {
    return 150 - (x + 29);
}

long test9(long x) {
    return (x - 30) - 25;
}

long test10(long x) {
    return (300 - x) - 50;
}

long test11(long x) {
    return 55 - (x - 40);
}

long test12(long x) {
    return 350 - (25 - x);
}

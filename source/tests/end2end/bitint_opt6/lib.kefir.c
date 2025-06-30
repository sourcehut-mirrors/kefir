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

_BitInt(60) get1(void) {
    return 3.14e2f;
}

unsigned _BitInt(60) get2(void) {
    return 3.14e2f;
}

_BitInt(60) get3(void) {
    return 2.71828e3;
}

unsigned _BitInt(60) get4(void) {
    return 2.71828e3;
}

_BitInt(60) get5(void) {
    return 2.71828e5L;
}

unsigned _BitInt(60) get6(void) {
    return 2.71828e5L;
}

_BitInt(60) get7(void) {
    return -3.14e2f;
}

unsigned _BitInt(60) get8(void) {
    return -3.14e2f;
}

_BitInt(60) get9(void) {
    return -2.71828e3;
}

unsigned _BitInt(60) get10(void) {
    return -2.71828e3;
}

_BitInt(60) get11(void) {
    return -2.71828e5L;
}

unsigned _BitInt(60) get12(void) {
    return -2.71828e5L;
}

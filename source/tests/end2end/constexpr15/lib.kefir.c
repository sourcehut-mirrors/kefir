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

constexpr unsigned _BitInt(6) a = 12uwb;
constexpr unsigned _BitInt(14) b = 1024uwb;
constexpr unsigned _BitInt(30) c = 0xcafeuwb;
constexpr unsigned _BitInt(60) d = 0xcafe0baduwb;
constexpr unsigned _BitInt(121) e = 0x123409876655aaaabbbbddddeeeeuwb;
constexpr unsigned _BitInt(250) f = 0x123409876655aaaabbbbddddeeeeuwb;

_BitInt(6) get1(void) {
    return a;
}

_BitInt(14) get2(void) {
    return b;
}

_BitInt(30) get3(void) {
    return c;
}

_BitInt(60) get4(void) {
    return d;
}

_BitInt(121) get5(void) {
    return e;
}

_BitInt(250) get6(void) {
    return f;
}

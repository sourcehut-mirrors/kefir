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

extern _Atomic unsigned _BitInt(6) a;
extern _Atomic unsigned _BitInt(13) b;
extern _Atomic unsigned _BitInt(27) c;
extern _Atomic unsigned _BitInt(50) d;
extern _Atomic unsigned _BitInt(111) e;
extern _Atomic unsigned _BitInt(366) f;

unsigned _BitInt(6) get1(void) {
    return a;
}

unsigned _BitInt(13) get2(void) {
    return b;
}

unsigned _BitInt(27) get3(void) {
    return c;
}

unsigned _BitInt(50) get4(void) {
    return d;
}

unsigned _BitInt(111) get5(void) {
    return e;
}

unsigned _BitInt(366) get6(void) {
    return f;
}

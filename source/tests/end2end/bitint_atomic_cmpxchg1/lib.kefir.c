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

extern _Atomic _BitInt(6) a;
extern _Atomic _BitInt(13) b;
extern _Atomic _BitInt(27) c;
extern _Atomic _BitInt(50) d;
extern _Atomic _BitInt(111) e;
extern _Atomic _BitInt(366) f;

_BitInt(6) add1(_BitInt(6) arg) {
    return a += arg;
}

_BitInt(13) add2(_BitInt(13) arg) {
    return b += arg;
}

_BitInt(27) add3(_BitInt(27) arg) {
    return c += arg;
}

_BitInt(50) add4(_BitInt(50) arg) {
    return d += arg;
}

_BitInt(111) add5(_BitInt(111) arg) {
    return e += arg;
}

_BitInt(366) add6(_BitInt(366) arg) {
    return f += arg;
}

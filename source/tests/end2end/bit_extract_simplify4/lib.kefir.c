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

short test1(unsigned _BitInt(14) x) {
    return (_BitInt(12)) x;
}

char test2(unsigned _BitInt(7) x) {
    return (_BitInt(5)) x;
}

char test3(unsigned _BitInt(60) x) {
    return (_BitInt(7))(unsigned _BitInt(9))(_BitInt(13))(unsigned _BitInt(20))(_BitInt(31))(unsigned _BitInt(42))(
        _BitInt(50)) x;
}

long test4(unsigned _BitInt(7) x) {
    return (_BitInt(60))(unsigned _BitInt(50))(_BitInt(37))(unsigned _BitInt(29))(_BitInt(14)) x;
}

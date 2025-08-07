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

unsigned short test1(_BitInt(14) x) {
    return (unsigned _BitInt(12)) x;
}

unsigned char test2(_BitInt(7) x) {
    return (unsigned _BitInt(5)) x;
}

unsigned char test3(_BitInt(60) x) {
    return (unsigned _BitInt(7))(_BitInt(9))(unsigned _BitInt(13))(_BitInt(20))(unsigned _BitInt(31))(_BitInt(42))(
        unsigned _BitInt(50)) x;
}

unsigned long test4(_BitInt(7) x) {
    return (unsigned _BitInt(60))(_BitInt(50))(unsigned _BitInt(37))(_BitInt(29))(unsigned _BitInt(14)) x;
}

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

int below1(unsigned _BitInt(6) x, unsigned _BitInt(6) y) {
    return x < y;
}

int below2(unsigned _BitInt(14) x, unsigned _BitInt(14) y) {
    return x < y;
}

int below3(unsigned _BitInt(29) x, unsigned _BitInt(29) y) {
    return x < y;
}

int below4(unsigned _BitInt(58) x, unsigned _BitInt(58) y) {
    return x < y;
}

int below5(unsigned _BitInt(120) x, unsigned _BitInt(120) y) {
    return x < y;
}

int below6(unsigned _BitInt(310) x, unsigned _BitInt(310) y) {
    return x < y;
}

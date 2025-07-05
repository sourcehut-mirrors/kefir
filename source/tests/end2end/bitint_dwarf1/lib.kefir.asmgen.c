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

_BitInt(7) x;
_BitInt(14) y;
_BitInt(24) z;
_BitInt(40) w;
_BitInt(120) a;
_BitInt(__BITINT_MAXWIDTH__) b;

_BitInt(90) get() {
    volatile _BitInt(9) r = 0wb;
    return r;
}

unsigned _BitInt(7) x2;
unsigned _BitInt(14) y2;
unsigned _BitInt(24) z2;
unsigned _BitInt(40) w2;
unsigned _BitInt(120) a2;
unsigned _BitInt(__BITINT_MAXWIDTH__) b2;

unsigned _BitInt(90) get2() {
    volatile unsigned _BitInt(9) r = 0uwb;
    return r;
}

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

_BitInt(6) get1(void) {
    return 0x123456789wb;
}

_BitInt(120) get2(void) {
    return 0x123wb;
}

_BitInt(6) get3(void) {
    return 0x123456779uwb;
}

_BitInt(120) get4(void) {
    return 0xabcbdbeb5362251uwb;
}

unsigned _BitInt(6) get5(void) {
    return 0x123456756wb;
}

unsigned _BitInt(120) get6(void) {
    return 0xaaaeeefff333222wb;
}

unsigned _BitInt(6) get7(void) {
    return 0x123456734uwb;
}

unsigned _BitInt(120) get8(void) {
    return 0x1230261371428417418uwb;
}

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

_BitInt(27) get0(void) {
    return 0xbad0cafe4e1e2ll;
}

_BitInt(27) get1(void) {
    return 0xbad0cafe4e1e2wb;
}

_BitInt(27) get2(void) {
    return 0x1234543wb;
}

unsigned _BitInt(27) get3(void) {
    return 0xeeffddeeffdd4uwb;
}

_BitInt(61) get4(void) {
    return 0xface4face6face7feace318217acedwb;
}

_BitInt(61) get5(void) {
    return 0x100000000000000000000000000000abcdefedwb;
}

unsigned _BitInt(61) get6(void) {
    return 0xaaaa1111222233334444dddd5555eeeffff77778888ccccuwb;
}

_BitInt(200) get7(void) {
    return (_BitInt(63)) - 152627362517262;
}

_BitInt(200) get8(void) {
    return 152627362517262wb;
}

unsigned _BitInt(200) get9(void) {
    return 0xc0ffe4e3e231uwb;
}

_BitInt(430) get10(void) {
    return (_BitInt(128)) - 8372472467146471642ll;
}

_BitInt(430) get11(void) {
    return 0x83724724671387248272875385728738346471642wb;
}

unsigned _BitInt(430) get12(void) {
    return 0xbabcbebdbcbbebdbdbcbebcbbdbcbac6462774637261ebdbcebde7uwb;
}
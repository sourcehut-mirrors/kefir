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
    return -3;
}

_BitInt(6) get1a(void) {
    return 3;
}

unsigned _BitInt(6) get2(void) {
    return 10;
}

_BitInt(14) get3(void) {
    return -4096;
}

_BitInt(14) get3a(void) {
    return 4096;
}

unsigned _BitInt(13) get4(void) {
    return 2048;
}

_BitInt(24) get5(void) {
    return -0xcafeb;
}

_BitInt(24) get5a(void) {
    return 0xcafeb;
}

unsigned _BitInt(24) get6(void) {
    return 0xff1ffu;
}

_BitInt(60) get7(void) {
    return -64327324284627417ll;
}

_BitInt(60) get7a(void) {
    return 64327324284627417ll;
}

_BitInt(60) get8(void) {
    return 0xfecefeefecull;
}

_BitInt(111) get9(void) {
    return -1028332837181762172ll;
}

_BitInt(111) get9a(void) {
    return 1028332837181762172ll;
}

unsigned _BitInt(111) get10(void) {
    return 0x376324623ebdef36ull;
}

_BitInt(350) get11(void) {
    return -1028332837181762172ll;
}

_BitInt(350) get11a(void) {
    return 1028332837181762172ll;
}

unsigned _BitInt(350) get12(void) {
    return 0x376324623ebdef36ull;
}

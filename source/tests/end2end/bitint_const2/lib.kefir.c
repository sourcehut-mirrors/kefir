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

signed _BitInt(6) get1(void) {
    return 30wb;
}

signed _BitInt(13) get2(void) {
    return 1024wb;
}

signed _BitInt(24) get3(void) {
    return 0xfffffewb;
}

signed _BitInt(50) get4(void) {
    return 0xcafebabe123wb;
}

signed _BitInt(120) get5(void) {
    return 0xfeca1234baad32109876543wb;
}

signed _BitInt(200) get6(void) {
    return 0xabcdef09871234567890987654321abcdefedbcawb;
}

signed _BitInt(1024) get7(void) {
    return 0xaabbccddeeff11223344556677889900aaabbbcccdddeeefff00009999888877776666wb;
}

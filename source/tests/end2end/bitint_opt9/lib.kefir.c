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

_BitInt(150) get1(void) {
    return ~(_BitInt(150)) 0x1234567890abcdefedfebce3245wb;
}

_BitInt(150) get2(void) {
    return ~(unsigned _BitInt(150)) 0x1234567890abcdefedfebce3245uwb;
}

_BitInt(150) get3(void) {
    return -(_BitInt(150)) 0x1234567890abcdefedfebce3245wb;
}

_BitInt(150) get4(void) {
    return -(unsigned _BitInt(150)) 0x1234567890abcdefedfebce3245uwb;
}

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

__CHAR8_TYPE__ a = u8'a';
__CHAR8_TYPE__ b = u8'\u00ab';
__CHAR8_TYPE__ c = u8'\u007e';
__CHAR8_TYPE__ d = u8'hello';

#define TEST1(x) u8##x
__CHAR8_TYPE__ e = TEST1('x');

__CHAR8_TYPE__ get1(void) {
    return u8'a';
}

__CHAR8_TYPE__ get2(void) {
    return u8'\u00ab';
}

__CHAR8_TYPE__ get3(void) {
    return u8'\u007e';
}

__CHAR8_TYPE__ get4(void) {
    return u8'hello';
}

__CHAR8_TYPE__ get5(void) {
    return TEST1('x');
}

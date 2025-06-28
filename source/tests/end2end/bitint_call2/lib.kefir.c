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

_BitInt(6) fn1(void);
_BitInt(14) fn2(void);
_BitInt(29) fn3(void);
_BitInt(60) fn4(void);
_BitInt(120) fn5(void);
_BitInt(360) fn6(void);

_BitInt(6) test1(void) {
    return fn1();
}

_BitInt(14) test2(void) {
    return fn2();
}

_BitInt(29) test3(void) {
    return fn3();
}

_BitInt(60) test4(void) {
    return fn4();
}

_BitInt(120) test5(void) {
    return fn5();
}

_BitInt(360) test6(void) {
    return fn6();
}

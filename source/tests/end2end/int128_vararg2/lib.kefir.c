/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

static __int128 testx(int x, ...) {
    __builtin_va_list args;
    __builtin_va_start(args, x);
    for (; x--;) {
        (void) __builtin_va_arg(args, int);
    }
    __int128 res = __builtin_va_arg(args, __int128);
    __builtin_va_end(args);
    return res;
}

__int128 test(int x) {
    return testx(7, 0, 0, 0, 0, 0, 0, 0, (__int128) x);
}

__int128 test2(int x) {
    return testx(6, 0, 0, 0, 0, 0, 0, 0, (__int128) x);
}

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

int test1(_BitInt(17) x) {
    return __builtin_clzg(x);
}

int test2(_BitInt(32) x) {
    return __builtin_clzg(x);
}

int test3(_BitInt(59) x) {
    return __builtin_clzg(x);
}

int test4(_BitInt(64) x) {
    return __builtin_clzg(x);
}

int test5(_BitInt(120) x) {
    return __builtin_clzg(x);
}

int test6(_BitInt(315) x) {
    return __builtin_clzg(x);
}

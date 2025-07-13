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

#define ASSERT_TYPE(_expr, _type) _Static_assert(_Generic(_expr, _type: 1, default: 0))

void fn1(void) {
    auto x = (unsigned short) 3;
    ASSERT_TYPE(x, int);
}

void fn2(void) {
    auto int x = (unsigned short) 3;
    ASSERT_TYPE(x, int);
}

void fn3(void) {
    auto long x = (unsigned short) 3;
    ASSERT_TYPE(x, long);
}

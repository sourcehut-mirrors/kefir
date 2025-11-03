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

static struct S {
        __int128 a : 32,
                   : 3,
                 b : 70;
} value = {
    0xbad0c0full,
    -0x81831bcde83131ll
};

__int128 get_a() {
        return value.a;
}

__int128 get_b() {
    return value.b;
}

void set_a(__int128 a) {
    value.a = a;
}

void set_b(__int128 b) {
    value.b = b;
}

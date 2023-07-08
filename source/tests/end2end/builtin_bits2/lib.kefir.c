/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

#include "./definitions.h"

int my_ffs(long x) {
    return __builtin_ffsl(x);
}

int my_clz(long x) {
    return __builtin_clzl(x);
}

int my_ctz(long x) {
    return __builtin_ctzl(x);
}

int my_clrsb(long x) {
    return __builtin_clrsbl(x);
}

int my_popcount(long x) {
    return __builtin_popcountl(x);
}

int my_parity(long x) {
    return __builtin_parityl(x);
}

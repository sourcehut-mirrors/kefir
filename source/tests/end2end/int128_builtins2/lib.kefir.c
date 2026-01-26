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

int int128_ffs(__int128 x) {
    return __builtin_ffsg(x);
}

int int128_clz(__int128 x, int y) {
    return __builtin_clzg(x, y);
}

int int128_ctz(__int128 x, int y) {
    return __builtin_ctzg(x, y);
}

int int128_clrsb(__int128 x) {
    return __builtin_clrsbg(x);
}

int int128_popcount(__int128 x) {
    return __builtin_popcountg(x);
}

int int128_parity(__int128 x) {
    return __builtin_parityg(x);
}

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

_Bool int128_add_overflow(__int128 x, __int128 y, __int128 *z) {
    return __builtin_add_overflow(x, y, z);
}

_Bool uint128_add_overflow(unsigned __int128 x, unsigned __int128 y, unsigned __int128 *z) {
    return __builtin_add_overflow(x, y, z);
}

_Bool int128_sub_overflow(__int128 x, __int128 y, __int128 *z) {
    return __builtin_sub_overflow(x, y, z);
}

_Bool uint128_sub_overflow(unsigned __int128 x, unsigned __int128 y, unsigned __int128 *z) {
    return __builtin_sub_overflow(x, y, z);
}

_Bool int128_mul_overflow(__int128 x, __int128 y, __int128 *z) {
    return __builtin_mul_overflow(x, y, z);
}

_Bool uint128_mul_overflow(unsigned __int128 x, unsigned __int128 y, unsigned __int128 *z) {
    return __builtin_mul_overflow(x, y, z);
}

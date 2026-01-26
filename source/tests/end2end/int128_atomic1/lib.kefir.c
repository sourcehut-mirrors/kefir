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

__int128 load_i128(_Atomic __int128 *ptr) {
    return *ptr;
}

unsigned __int128 load_u128(_Atomic unsigned __int128 *ptr) {
    return *ptr;
}

void store_i128(_Atomic __int128 *ptr, __int128 value) {
    *ptr = value;
}

void store_u128(_Atomic unsigned __int128 *ptr, unsigned __int128 value) {
    *ptr = value;
}

__int128 add_i128(_Atomic __int128 *ptr, __int128 value) {
    return *ptr += value;
}

unsigned __int128 add_u128(_Atomic unsigned __int128 *ptr, unsigned __int128 value) {
    return *ptr += value;
}

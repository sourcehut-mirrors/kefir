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

__int128 i128_neg(__int128 a) {
    return -a;
}

unsigned __int128 u128_neg(unsigned __int128 a) {
    return -a;
}

__int128 i128_not(__int128 a) {
    return ~a;
}

unsigned __int128 u128_not(unsigned __int128 a) {
    return ~a;
}

int i128_bnot(__int128 a) {
    return !a;
}

int u128_bnot(unsigned __int128 a) {
    return !a;
}

__int128 i128_preinc(__int128 *a) {
    return ++(*a);
}

__int128 i128_predec(__int128 *a) {
    return --(*a);
}

__int128 i128_postinc(__int128 *a) {
    return (*a)++;
}

__int128 i128_postdec(__int128 *a) {
    return (*a)--;
}

unsigned __int128 u128_preinc(unsigned __int128 *a) {
    return ++(*a);
}

unsigned __int128 u128_predec(unsigned __int128 *a) {
    return --(*a);
}

unsigned __int128 u128_postinc(unsigned __int128 *a) {
    return (*a)++;
}

unsigned __int128 u128_postdec(unsigned __int128 *a) {
    return (*a)--;
}

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

__int128 i128_from_f32(float x) {
    return x;
}

__int128 i128_from_f64(double x) {
    return x;
}

__int128 i128_from_f80(long double x) {
    return x;
}

unsigned __int128 u128_from_f32(float x) {
    return x;
}

unsigned __int128 u128_from_f64(double x) {
    return x;
}

unsigned __int128 u128_from_f80(long double x) {
    return x;
}

__int128  i128_from_i9(_BitInt(9) x) {
    return x;   
}

__int128  i128_from_u9(unsigned _BitInt(9) x) {
    return x;   
}

unsigned __int128 u128_from_i9(_BitInt(9) x) {
    return x;   
}

unsigned __int128 u128_from_u9(unsigned _BitInt(9) x) {
    return x;   
}

__int128  i128_from_i256(_BitInt(256) x) {
    return x;   
}

__int128  i128_from_u256(unsigned _BitInt(256) x) {
    return x;   
}

unsigned __int128 u128_from_i256(_BitInt(256) x) {
    return x;   
}

unsigned __int128 u128_from_u256(unsigned _BitInt(256) x) {
    return x;   
}

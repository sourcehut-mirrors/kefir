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

__int128 int128_zero_extend64(void) {
    return (unsigned long) -0xcafebabell;
}

__int128 int128_sign_extend64(void) {
    return (long) -0xcafebabell;
}

unsigned long int128_truncate_int64(void) {
    return (__int128) (~0ull << 32);
}

_Bool int128_to_bool(void) {
    return (__int128) (~0ull << 64);
}

_Bool int128_bool_not(void) {
    return !((__int128) ~0ull);
}

__int128 int128_neg(void) {
    return -(__int128) 0x0bad0c0ffeell;
}

__int128 int128_not(void) {
    return ~(__int128) 0x0bad0c0ffeell;
}

_BitInt(192) int128_sign_to_bitint(void) {
    return (__int128) -0x0bad0c0ffeell;
}

_BitInt(192) int128_unsign_to_bitint(void) {
    return (unsigned __int128) -0x0bad0c0ffeell;
}

__int128 int128_from_bitint_sign(void) {
    return (_BitInt(192)) -0x0bad0c0ffeell;
}

__int128 int128_from_bitint_unsign(void) {
    return (unsigned _BitInt(192)) 0x0bad0c0ffeell;
}

float int128_signed_to_float(void) {
    return (__int128) -314159;
}

float int128_unsigned_to_float(void) {
    return (unsigned __int128) 314159;
}

double int128_signed_to_double(void) {
    return (__int128) -314159;
}

double int128_unsigned_to_double(void) {
    return (unsigned __int128) 314159;
}

long double int128_signed_to_long_double(void) {
    return (__int128) -314159;
}

long double int128_unsigned_to_long_double(void) {
    return (unsigned __int128) 314159;
}

__int128 int128_signed_from_float(void) {
    return -3.14159e4f;
}

unsigned __int128 int128_unsigned_from_float(void) {
    return 3.14159e4f;
}

__int128 int128_signed_from_double(void) {
    return -3.14159e4;
}

unsigned __int128 int128_unsigned_from_double(void) {
    return 3.14159e4;
}

__int128 int128_signed_from_long_double(void) {
    return -3.14159e4l;
}

unsigned __int128 int128_unsigned_from_long_double(void) {
    return 3.14159e4l;
}

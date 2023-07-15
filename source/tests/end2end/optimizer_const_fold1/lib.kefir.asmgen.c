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

int int_add(void) {
    return 2 + 3 + (4 + 5);
}

int int_sub(void) {
    return 0 - 100 - 55 - (1 - 6);
}

int int_sub2(void) {
    return 0ULL - 1;
}

int int_mul(void) {
    return 100 * 2 * (5 * 3);
}

int int_div(void) {
    return 100 / (0 - 2);
}

int int_div2(void) {
    return 100u / (0 - 2);
}

int int_div3(void) {
    return 100u / (24 / 6);
}

int int_mod(void) {
    return 180 % 23;
}

int int_mod2(void) {
    return -180 % 23;
}

int int_mod3(void) {
    return (~0ull) % (~0ull >> 31);
}

long int_and(void) {
    return 0xcafebabeul & 0xdeadc0feul;
}

long int_or(void) {
    return 0xcafebabeul | 0xdeadc0feul;
}

long int_xor(void) {
    return 0xcafebabeul ^ 0xdeadc0feul;
}

int int_shl(void) {
    return 1024 << 2;
}

unsigned long int_shr(void) {
    return (~0ull) >> 2;
}

unsigned long int_ashr(void) {
    return (~0ll) >> 2;
}

unsigned long int_ashr2(void) {
    return (-1024) >> 2;
}

int int_equals(void) {
    return (2 + 2 * 2) == (3 + 2 + 1);
}

int int_equals2(void) {
    return (2 + 2 * 2) == (4 + 2 + 1);
}

int int_equals3(void) {
    return (2 + 2 * 2) == (3 + 2 + 1);
}

int int_greater(void) {
    return -3 > 10;
}

int int_greater2(void) {
    return -3 > -10;
}

int int_greater_or_equals(void) {
    return 100 >= (99 + 2 / 2);
}

int int_greater_or_equals2(void) {
    return -1 >= (99 + 2 / 2);
}

int int_less(void) {
    return -3 < 10;
}

int int_less2(void) {
    return -3 < -10;
}

int int_less_or_equals(void) {
    return 100 <= (99 + 2 / 2);
}

int int_less_or_equals2(void) {
    return 1000 <= (99 + 2 / 2);
}

int int_above(void) {
    return -3 > 10u;
}

int int_above2(void) {
    return 3 > 10u;
}

int int_above_or_equals(void) {
    return -3 >= (-1 * (2 + 1));
}

int int_above_or_equals2(void) {
    return -10 >= (-1 * (2 + 1));
}

int int_below(void) {
    return -3 < 10u;
}

int int_below2(void) {
    return 3 < 10u;
}

int int_below_or_equals(void) {
    return -3 <= (0ull - (4 - 1));
}

int int_below_or_equals2(void) {
    return -1 <= (0ull - (4 - 1));
}

int int_neg(void) {
    return -1234;
}

int int_not(void) {
    return ~1234;
}

_Bool truct1bit(void) {
    return (_Bool) 0xcafebabe;
}

_Bool truct1bit2(void) {
    return (_Bool) 0;
}

signed char sign_extend8(void) {
    return 256 + 128;
}

unsigned char zero_extend8(void) {
    return 256 + 128;
}

short sign_extend16(void) {
    return 0xffff + 0x8fff;
}

unsigned short zero_extend16(void) {
    return 0xffff + 0x8fff;
}

int sign_extend32(void) {
    return 0xffffffffffff + 0x9fffffffff;
}

unsigned int zero_extend32(void) {
    return 0xffffffffffff + 0x9fffffffff;
}

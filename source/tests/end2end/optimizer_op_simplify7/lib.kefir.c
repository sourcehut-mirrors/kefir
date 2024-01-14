/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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

long int_add(long x) {
    return 100 + (x - 50 + 5) - 400;
}

long int_sub(long x) {
    return x - 5 - 10 - 15 + 20 - 1;
}

long int_sub2(long x) {
    return x - 5 - 10 - 15 - 20 - 1;
}

long int_mul(long x) {
    return 2 * x * 10 * 5;
}

long int_and(long x) {
    return 0xfefefe & x & 0xcacaca & 0xfe;
}

long int_or(long x) {
    return 0x12012 | x | 0x100000 | 0x1;
}

long int_xor(long x) {
    return 0x10203040 ^ x ^ 0xfefefe3 ^ 0x11;
}

unsigned long uint_and(unsigned long x) {
    return 0xfefefeu & x & 0xcacacau & 0xfeu;
}

unsigned long uint_or(unsigned long x) {
    return 0x12012u | x | 0x100000u | 0x1u;
}

unsigned long uint_xor(unsigned long x) {
    return 0x10203040u ^ x ^ 0xfefefe3u ^ 0x11u;
}

unsigned long uint_add(unsigned long x) {
    return 100u + (x - 50u + 5u) - 400u;
}

unsigned long uint_sub(unsigned long x) {
    return x - 5u - 10u - 15u + 20u - 1u;
}

unsigned long uint_sub2(unsigned long x) {
    return x - 5u - 10u - 15u - 20u - 1u;
}

unsigned long uint_mul(unsigned long x) {
    return 2u * x * 10u * 5u;
}

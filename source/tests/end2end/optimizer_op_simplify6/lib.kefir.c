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

long int_add(long x) {
    return (1 + x) * (x + (12345 - 1));
}

long int_sub(long x) {
    return x - (0 - 100);
}

long int_mul(long x) {
    return (2 + 3) * x + x * 6;
}

long int_and(long x) {
    return (-381 & x) | (x & 12356);
}

long int_or(long x) {
    return (10291 | x) & (x | 81712);
}

long int_xor(long x) {
    return (x ^ 76) + (7894 ^ x);
}

unsigned long uint_add(unsigned long x) {
    return (1ul + x) * (x + (12345ul - 1ul));
}

unsigned long uint_sub(unsigned long x) {
    return x - 257ul;
}

unsigned long uint_mul(unsigned long x) {
    return (2ul + 3ul) * x + x * 6ul;
}

unsigned long uint_and(unsigned long x) {
    return (381ul & x) | (x & 12356ul);
}

unsigned long uint_or(unsigned long x) {
    return (10291ul | x) & (x | 81712ul);
}

unsigned long uint_xor(unsigned long x) {
    return (x ^ 76ul) + (7894ul ^ x);
}

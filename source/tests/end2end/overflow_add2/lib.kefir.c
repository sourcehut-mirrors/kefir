/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "./definitions.h"

_Bool overflow_sadd(int a, int b, int *r) {
    return __builtin_sadd_overflow(a, b, r);
}

_Bool overflow_saddl(long int a, long int b, long int *r) {
    return __builtin_saddl_overflow(a, b, r);
}

_Bool overflow_saddll(long long int a, long long int b, long long int *r) {
    return __builtin_saddll_overflow(a, b, r);
}

_Bool overflow_uadd(unsigned int a, unsigned int b, unsigned int *r) {
    return __builtin_uadd_overflow(a, b, r);
}

_Bool overflow_uaddl(unsigned long int a, unsigned long int b, unsigned long int *r) {
    return __builtin_uaddl_overflow(a, b, r);
}

_Bool overflow_uaddll(unsigned long long int a, unsigned long long int b, unsigned long long int *r) {
    return __builtin_uaddll_overflow(a, b, r);
}

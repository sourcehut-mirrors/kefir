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

#include "./definitions.h"

_Atomic long double ld1 = 0.0L;
_Atomic _Complex long double cld1 = 0.0L;

void add_ld1(long double x) {
    long double current = __atomic_load_n(&ld1, __ATOMIC_ACQUIRE);
    while (!__atomic_compare_exchange_n(&ld1, &current, current + x, 0, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE))
        ;
}

void add_cld1(_Complex long double x) {
    _Complex long double current = __atomic_load_n(&cld1, __ATOMIC_ACQUIRE);
    while (!__atomic_compare_exchange_n(&cld1, &current, current + x, 0, __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE))
        ;
}

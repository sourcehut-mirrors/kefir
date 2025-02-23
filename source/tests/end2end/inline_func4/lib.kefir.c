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

inline long load(_Atomic long *ptr) {
    return *ptr;
}

inline _Bool cmp_xchg(_Atomic long *ptr, long *expected, long *desired) {
    return __atomic_compare_exchange(ptr, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

long addx(long y) {
    long xval = load(&x), new_xval;
    do {
        new_xval = xval + y;
    } while (!cmp_xchg(&x, &xval, &new_xval));
    return new_xval;
}

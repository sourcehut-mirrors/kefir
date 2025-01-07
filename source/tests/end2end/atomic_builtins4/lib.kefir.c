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

_Bool test_atomic_compare_exchange8(_Atomic char *ptr, char *expected, char desired) {
    return __atomic_compare_exchange(ptr, expected, &desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

_Bool test_atomic_compare_exchange16(_Atomic short *ptr, short *expected, short desired) {
    return __atomic_compare_exchange(ptr, expected, &desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

_Bool test_atomic_compare_exchange32(_Atomic int *ptr, int *expected, int desired) {
    return __atomic_compare_exchange(ptr, expected, &desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

_Bool test_atomic_compare_exchange64(_Atomic long *ptr, long *expected, long desired) {
    return __atomic_compare_exchange(ptr, expected, &desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

_Bool test2_atomic_compare_exchange8(_Atomic char *ptr, char *expected, char desired) {
    return __atomic_compare_exchange_n(ptr, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

_Bool test2_atomic_compare_exchange16(_Atomic short *ptr, short *expected, short desired) {
    return __atomic_compare_exchange_n(ptr, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

_Bool test2_atomic_compare_exchange32(_Atomic int *ptr, int *expected, int desired) {
    return __atomic_compare_exchange_n(ptr, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

_Bool test2_atomic_compare_exchange64(_Atomic long *ptr, long *expected, long desired) {
    return __atomic_compare_exchange_n(ptr, expected, desired, 0, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}

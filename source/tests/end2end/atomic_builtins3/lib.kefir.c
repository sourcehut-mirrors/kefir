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

char test_atomic_exchange8(_Atomic char *ptr, char value) {
    __typeof__(value) prev;
    __atomic_exchange(ptr, &value, &prev, __ATOMIC_SEQ_CST);
    return prev;
}

short test_atomic_exchange16(_Atomic short *ptr, short value) {
    __typeof__(value) prev;
    __atomic_exchange(ptr, &value, &prev, __ATOMIC_SEQ_CST);
    return prev;
}

int test_atomic_exchange32(_Atomic int *ptr, int value) {
    __typeof__(value) prev;
    __atomic_exchange(ptr, &value, &prev, __ATOMIC_SEQ_CST);
    return prev;
}

long test_atomic_exchange64(_Atomic long *ptr, long value) {
    __typeof__(value) prev;
    __atomic_exchange(ptr, &value, &prev, __ATOMIC_SEQ_CST);
    return prev;
}

char test2_atomic_exchange8(_Atomic char *ptr, char value) {
    return __atomic_exchange_n(ptr, value, __ATOMIC_SEQ_CST);
}

short test2_atomic_exchange16(_Atomic short *ptr, short value) {
    return __atomic_exchange_n(ptr, value, __ATOMIC_SEQ_CST);
}

int test2_atomic_exchange32(_Atomic int *ptr, int value) {
    return __atomic_exchange_n(ptr, value, __ATOMIC_SEQ_CST);
}

long test2_atomic_exchange64(_Atomic long *ptr, long value) {
    return __atomic_exchange_n(ptr, value, __ATOMIC_SEQ_CST);
}

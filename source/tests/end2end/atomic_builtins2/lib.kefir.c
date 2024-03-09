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

char test_atomic_load8(_Atomic char *ptr) {
    __typeof_unqual__(*ptr) value;
    __atomic_load(ptr, &value, __ATOMIC_SEQ_CST);
    return value;
}

short test_atomic_load16(_Atomic short *ptr) {
    __typeof_unqual__(*ptr) value;
    __atomic_load(ptr, &value, __ATOMIC_SEQ_CST);
    return value;
}

int test_atomic_load32(_Atomic int *ptr) {
    __typeof_unqual__(*ptr) value;
    __atomic_load(ptr, &value, __ATOMIC_SEQ_CST);
    return value;
}

long test_atomic_load64(_Atomic long *ptr) {
    __typeof_unqual__(*ptr) value;
    __atomic_load(ptr, &value, __ATOMIC_SEQ_CST);
    return value;
}

long double test_atomic_load128(_Atomic long double *ptr) {
    __typeof_unqual__(*ptr) value;
    __atomic_load(ptr, &value, __ATOMIC_SEQ_CST);
    return value;
}

_Complex long double test_atomic_load256(_Atomic _Complex long double *ptr) {
    __typeof_unqual__(*ptr) value;
    __atomic_load(ptr, &value, __ATOMIC_SEQ_CST);
    return value;
}

char test2_atomic_load8(_Atomic char *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}

short test2_atomic_load16(_Atomic short *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}

int test2_atomic_load32(_Atomic int *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}

long test2_atomic_load64(_Atomic long *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}

long double test2_atomic_load128(_Atomic long double *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}

_Complex long double test2_atomic_load256(_Atomic _Complex long double *ptr) {
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}

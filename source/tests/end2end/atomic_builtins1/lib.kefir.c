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

void test_atomic_store8(_Atomic char *ptr, char val) {
    __atomic_store(ptr, &val, __ATOMIC_SEQ_CST);
}

void test_atomic_store16(_Atomic short *ptr, short val) {
    __atomic_store(ptr, &val, __ATOMIC_SEQ_CST);
}

void test_atomic_store32(_Atomic int *ptr, int val) {
    __atomic_store(ptr, &val, __ATOMIC_SEQ_CST);
}

void test_atomic_store64(_Atomic long *ptr, long val) {
    __atomic_store(ptr, &val, __ATOMIC_SEQ_CST);
}

void test_atomic_store128(_Atomic long double *ptr, long double val) {
    __atomic_store(ptr, &val, __ATOMIC_SEQ_CST);
}

void test_atomic_store256(_Atomic _Complex long double *ptr, _Complex long double val) {
    __atomic_store(ptr, &val, __ATOMIC_SEQ_CST);
}

void test2_atomic_store8(_Atomic char *ptr, char val) {
    __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST);
}

void test2_atomic_store16(_Atomic short *ptr, short val) {
    __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST);
}

void test2_atomic_store32(_Atomic int *ptr, int val) {
    __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST);
}

void test2_atomic_store64(_Atomic long *ptr, long val) {
    __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST);
}

void test2_atomic_store128(_Atomic long double *ptr, long double val) {
    __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST);
}

void test2_atomic_store256(_Atomic _Complex long double *ptr, _Complex long double val) {
    __atomic_store_n(ptr, val, __ATOMIC_SEQ_CST);
}

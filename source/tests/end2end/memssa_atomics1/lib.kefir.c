/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

int test1(int *xptr) {
    int x = *xptr;
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    int y = *xptr;
    return x + y;
}

int test2(int *xptr) {
    int x = *xptr;
    __atomic_signal_fence(__ATOMIC_SEQ_CST);
    int y = *xptr;
    return x + y;
}

int test3(int *xptr) {
    static int z;
    int x = *xptr;
    __atomic_store(&z, &(int) {1}, __ATOMIC_SEQ_CST);
    int y = *xptr;
    return x + y;
}

int test4(int *xptr) {
    static int z;
    int x = *xptr;
    __atomic_store_n(&z, 1, __ATOMIC_SEQ_CST);
    int y = *xptr;
    return x + y;
}

int test5(int *xptr) {
    static int z;
    int a;
    int x = *xptr;
    __atomic_load(&z, &a, __ATOMIC_SEQ_CST);
    int y = *xptr;
    return x + y;
}

int test6(int *xptr) {
    static int z;
    int x = *xptr;
    (void) __atomic_load_n(&z, __ATOMIC_SEQ_CST);
    int y = *xptr;
    return x + y;
}

int test7(int *xptr) {
    static int z;
    int x = *xptr;
    __atomic_fetch_add(&z, x, __ATOMIC_SEQ_CST);
    int y = *xptr;
    return x + y;
}
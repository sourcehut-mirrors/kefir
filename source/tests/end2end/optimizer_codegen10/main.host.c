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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include "./definitions.h"

_Thread_local long Var1 = 0;

void *thread_fn(void *arg) {
    (void) arg;
    assert(Var1 == 0);
    assert(Var2 == 0);
    set_var1(get_var1() + 100);
    set_var2(get_var2() + 300);
    assert(Var1 == 100);
    assert(Var2 == 300);
    return NULL;
}

int main(void) {
    for (long i = -100; i < 100; i++) {
        for (long j = -100; j < 100; j++) {
            Var1 = i * j;
            Var2 = i - j;
            assert(get_var1() == i * j);
            assert(get_var2() == i - j);

            set_var1(i + j);
            set_var2(i ^ j);
            assert(Var1 == i + j);
            assert(Var2 == (i ^ j));
        }
    }

    set_var1(123);
    set_var2(673);

    pthread_t thread1;
    assert(pthread_create(&thread1, NULL, thread_fn, NULL) == 0);
    assert(pthread_join(thread1, NULL) == 0);

    assert(get_var1() == 123);
    assert(get_var2() == 673);
    return EXIT_SUCCESS;
}

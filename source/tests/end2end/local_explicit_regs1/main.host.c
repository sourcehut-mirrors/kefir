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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./definitions.h"

long somefn(long a, long b, long c) {
    return a * b + c;
}

int main(void) {
#ifdef __x86_64__
    for (long i = -10; i < 10; i++) {
        for (long j = -100; j < 100; j++) {
            for (long k = -100; k < 100; k++) {
                long res = testfn(i, j, k);
                assert(res == somefn(k, i, j));
            }
        }   
    }
#endif
    return EXIT_SUCCESS;
}

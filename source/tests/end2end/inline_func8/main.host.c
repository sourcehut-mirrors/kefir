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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./definitions.h"

static inline unsigned long own_fibonacci(unsigned long n) {
    if (n <= 1) {
        return n;
    }
    return own_fibonacci(n - 1) + own_fibonacci(n - 2);
}

int main(void) {
    for (unsigned long a = 0; a <= 38; a++) {
        assert(fib(a) == own_fibonacci(a));
    }
    return EXIT_SUCCESS;
}

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
#include "./definitions.h"

#ifdef __x86_64__
extern long factorial(long);
#endif

long fact2(long x) {
    if (x <= 1) {
        return 1;
    }
    return x * fact2(x - 1);
}

int main(void) {
#ifdef __x86_64__
    for (long x = 0; x <= 15; x++) {
        assert(factorial(x) == fact2(x));
    }
#endif
    return EXIT_SUCCESS;
}

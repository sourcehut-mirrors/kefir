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

int main(void) {
    for (long x = -100; x < 100; x++) {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
#elif defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-void-pointer-cast"
#endif
        assert(char_to_ptr(x) == (void *) x);
        assert(short_to_ptr(x) == (void *) x);
        assert(int_to_ptr(x) == (void *) x);
        assert(long_to_ptr(x) == (void *) x);

        assert(unsigned_char_to_ptr(x) == (void *) (unsigned char) x);
        assert(unsigned_short_to_ptr(x) == (void *) (unsigned short) x);
        assert(unsigned_int_to_ptr(x) == (void *) (unsigned int) x);
        assert(unsigned_long_to_ptr(x) == (void *) (unsigned long) x);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
    }
    return EXIT_SUCCESS;
}

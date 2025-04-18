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
#include <assert.h>
#include "./definitions.h"

#define DEF(_size)                         \
    struct X##_size call##_size(long x) {  \
        struct X##_size s = {0};           \
        for (long i = 0; i < _size; i++) { \
            s.content[i] = i ^ x;          \
        }                                  \
        return s;                          \
    }

DEF(1)
DEF(2)
DEF(3)
DEF(4)
DEF(5)
DEF(6)
DEF(7)
DEF(8)

int main(void) {
    for (long x = -1000; x < 1000; x++) {
        struct X1 s1 = call1_proxy(x);
        struct X2 s2 = call2_proxy(x);
        struct X3 s3 = call3_proxy(x);
        struct X4 s4 = call4_proxy(x);
        struct X5 s5 = call5_proxy(x);
        struct X6 s6 = call6_proxy(x);
        struct X7 s7 = call7_proxy(x);
        struct X8 s8 = call8_proxy(x);

        assert(s1.content[0] == (char) x);
        assert(s2.content[0] == (char) x && s2.content[1] == (char) (1 ^ x));
        assert(s3.content[0] == (char) x && s3.content[1] == (char) (1 ^ x) && s3.content[2] == (char) (2 ^ x));
        assert(s4.content[0] == (char) x && s4.content[1] == (char) (1 ^ x) && s4.content[2] == (char) (2 ^ x) &&
               s4.content[3] == (char) (3 ^ x));
        assert(s5.content[0] == (char) x && s5.content[1] == (char) (1 ^ x) && s5.content[2] == (char) (2 ^ x) &&
               s5.content[3] == (char) (3 ^ x) && s5.content[4] == (char) (4 ^ x));
        assert(s6.content[0] == (char) x && s6.content[1] == (char) (1 ^ x) && s6.content[2] == (char) (2 ^ x) &&
               s6.content[3] == (char) (3 ^ x) && s6.content[4] == (char) (4 ^ x) && s6.content[5] == (char) (5 ^ x));
        assert(s7.content[0] == (char) x && s7.content[1] == (char) (1 ^ x) && s7.content[2] == (char) (2 ^ x) &&
               s7.content[3] == (char) (3 ^ x) && s7.content[4] == (char) (4 ^ x) && s7.content[5] == (char) (5 ^ x) &&
               s7.content[6] == (char) (6 ^ x));
        assert(s8.content[0] == (char) x && s8.content[1] == (char) (1 ^ x) && s8.content[2] == (char) (2 ^ x) &&
               s8.content[3] == (char) (3 ^ x) && s8.content[4] == (char) (4 ^ x) && s8.content[5] == (char) (5 ^ x) &&
               s8.content[6] == (char) (6 ^ x) && s8.content[7] == (char) (7 ^ x));
    }
    return EXIT_SUCCESS;
}

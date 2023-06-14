/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
#include "./definitions.h"

int main(void) {
#define BEGIN -25
#define END 25
    for (int a = BEGIN; a <= END; a++) {
        for (int b = BEGIN; b <= END; b++) {
            for (int c = BEGIN; c <= END; c++) {
#define ASSERT_TYPE(_type, a, b, c)                                     \
    assert(neg_discriminant_##_type((_type) a, (_type) b, (_type) c) == \
           (_type) NEG_DISCRIMINANT((_type) a, (_type) b, (_type) c))

                ASSERT_TYPE(char, a, b, c);
                ASSERT_TYPE(uchar, a, b, c);
                ASSERT_TYPE(short, a, b, c);
                ASSERT_TYPE(ushort, (a + (-BEGIN)), (b + (-BEGIN)), (c + (-BEGIN)));
                ASSERT_TYPE(int, a, b, c);
                ASSERT_TYPE(uint, (a + (-BEGIN)), (b + (-BEGIN)), (c + (-BEGIN)));
                ASSERT_TYPE(long, a, b, c);
                ASSERT_TYPE(ulong, (a + (-BEGIN)), (b + (-BEGIN)), (c + (-BEGIN)));
                ASSERT_TYPE(llong, a, b, c);
                ASSERT_TYPE(ullong, (a + (-BEGIN)), (b + (-BEGIN)), (c + (-BEGIN)));
                ASSERT_TYPE(llong, a * -100, b * 250, c * 10000);
                ASSERT_TYPE(ullong, a + 0xbadcafe, (b + (-BEGIN)) * 250, (c + (-BEGIN)) * 0xfefec);
            }
        }
    }
    return EXIT_SUCCESS;
}

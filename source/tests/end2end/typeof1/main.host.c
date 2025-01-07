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
#include <stdbool.h>
#include "./definitions.h"

static _Bool callback_flag = false;
int test_callback(int x) {
    callback_flag = true;
    return x + 1;
}

int main(void) {
    struct Str1 str1;
    test1(&str1);

    assert(str1.char_sz == sizeof(char));
    assert(str1.char_align == _Alignof(char));
    assert(str1.short_sz == sizeof(short));
    assert(str1.short_align == _Alignof(short));
    assert(str1.int_sz == sizeof(int));
    assert(str1.int_align == _Alignof(int));
    assert(str1.long_sz == sizeof(long));
    assert(str1.long_align == _Alignof(long));
    assert(str1.float_sz == sizeof(float));
    assert(str1.float_align == _Alignof(float));
    assert(str1.double_sz == sizeof(double));
    assert(str1.double_align == _Alignof(double));
    assert(str1.long_double_sz == sizeof(long double));
    assert(str1.long_double_align == _Alignof(long double));
    assert(str1.complex_float_sz == sizeof(_Complex float));
    assert(str1.complex_float_align == _Alignof(_Complex float));
    assert(str1.complex_double_sz == sizeof(_Complex double));
    assert(str1.complex_double_align == _Alignof(_Complex double));
    assert(str1.complex_long_double_sz == sizeof(_Complex long double));
    assert(str1.complex_long_double_align == _Alignof(_Complex long double));
    assert(str1.str1_sz == sizeof(struct Str1));
    assert(str1.str1_align == _Alignof(struct Str1));

    for (int i = 0; i < 20; i++) {
        callback_flag = false;
        assert((unsigned long) test3(i) == sizeof(long) * (i + 1));
        assert(callback_flag);
    }

    assert(test4() == sizeof(somearr));
    assert((unsigned long) test5() == sizeof(int) * 20 * 30 * 40);
    return EXIT_SUCCESS;
}

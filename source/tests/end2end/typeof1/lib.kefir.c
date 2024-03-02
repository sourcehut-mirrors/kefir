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

void test1(struct Str1 *str) {
    __typeof__((char) '\0') chr;
    __typeof__((short) 254) shrt;
    __typeof__(52 + 3) integer;
    __typeof__(120l - 54l) lng;
    __typeof__(3.14159f) flt;
    __typeof__(2.718281828) dbl;
    __typeof__(69.69L) ldbl;
    __typeof__((_Complex float) 3.14f) cflt;
    __typeof__((_Complex double) 3.14) cdbl;
    __typeof__((_Complex long double) 3.14) cldbl;
    __typeof__(*str) str1;

    str->char_sz = sizeof(chr);
    str->char_align = _Alignof(chr);
    str->short_sz = sizeof(shrt);
    str->short_align = _Alignof(shrt);
    str->int_sz = sizeof(integer);
    str->int_align = _Alignof(integer);
    str->long_sz = sizeof(lng);
    str->long_align = _Alignof(lng);
    str->float_sz = sizeof(flt);
    str->float_align = _Alignof(flt);
    str->double_sz = sizeof(dbl);
    str->double_align = _Alignof(dbl);
    str->long_double_sz = sizeof(ldbl);
    str->long_double_align = _Alignof(ldbl);
    str->complex_float_sz = sizeof(cflt);
    str->complex_float_align = _Alignof(cflt);
    str->complex_double_sz = sizeof(cdbl);
    str->complex_double_align = _Alignof(cdbl);
    str->complex_long_double_sz = sizeof(cldbl);
    str->complex_long_double_align = _Alignof(cldbl);
    str->str1_sz = sizeof(str1);
    str->str1_align = _Alignof(str1);
}

void test2(const int x) {
    volatile __typeof_unqual__(x) y;
    y = x + 5;
}

int test3(int x) {
    __typeof__(long[test_callback(x)]) array;
    return sizeof(array);
}

int test4() {
    __typeof__(somearr) arr2;
    return sizeof(arr2);
}

int test5() {
    __typeof__(int[20][30][40]) arr;
    return sizeof(arr);
}

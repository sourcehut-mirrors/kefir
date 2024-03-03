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
    __auto_type chr = (char) '\0';
    __auto_type shrt = (short) 254;
    __auto_type integer = 52 + 3;
    __auto_type lng = 120l - 54l;
    __auto_type flt = 3.14159f;
    __auto_type dbl = 2.718281828;
    __auto_type ldbl = 69.69L;
    __auto_type cflt = (_Complex float) 3.14f;
    __auto_type cdbl = (_Complex double) 3.14;
    __auto_type cldbl = (_Complex long double) 3.14;
    __auto_type str1 = *str;

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
    volatile __auto_type y = x;
    y = x + 5;
}

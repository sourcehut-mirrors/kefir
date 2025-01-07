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

#include "./definitions.h"
#include <typeclass.h>

extern void abort(void);

enum Enum1 { A = 1 };

struct Struct1 {
    int x;
};

union Union1 {
    int x;
};

int arr[] = {__builtin_classify_type((void) 0),
             __builtin_classify_type((char) 0),
             __builtin_classify_type((signed char) 0),
             __builtin_classify_type((unsigned char) 0),
             __builtin_classify_type((short) 0),
             __builtin_classify_type((unsigned short) 0),
             __builtin_classify_type((int) 0),
             __builtin_classify_type((unsigned int) 0),
             __builtin_classify_type((long) 0),
             __builtin_classify_type((unsigned long) 0),
             __builtin_classify_type((long long) 0),
             __builtin_classify_type((unsigned long long) 0),
             __builtin_classify_type((long *) 0),
             __builtin_classify_type((enum Enum1) 0),
             __builtin_classify_type((long[]){0}),
             __builtin_classify_type(abort),
             __builtin_classify_type((struct Struct1){0}),
             __builtin_classify_type((union Union1){0})};

int arr2[] = {void_type_class,     char_type_class,    char_type_class,    char_type_class,     integer_type_class,
              integer_type_class,  integer_type_class, integer_type_class, integer_type_class,  integer_type_class,
              integer_type_class,  integer_type_class, pointer_type_class, enumeral_type_class, array_type_class,
              function_type_class, record_type_class,  union_type_class};

int arr_len = sizeof(arr) / sizeof(arr[0]);

int test(int x) {
    __builtin_choose_expr(1, (void) 0, abort());
    switch (x) {
        case 0:
            return __builtin_classify_type((void) 0);
        case 1:
            return __builtin_classify_type((char) 0);
        case 2:
            return __builtin_classify_type((signed char) 0);
        case 3:
            return __builtin_classify_type((unsigned char) 0);
        case 4:
            return __builtin_classify_type((short) 0);
        case 5:
            return __builtin_classify_type((unsigned short) 0);
        case 6:
            return __builtin_classify_type((int) 0);
        case 7:
            return __builtin_classify_type((unsigned int) 0);
        case 8:
            return __builtin_classify_type((long) 0);
        case 9:
            return __builtin_classify_type((unsigned long) 0);
        case 10:
            return __builtin_classify_type((long long) 0);
        case 11:
            return __builtin_classify_type((unsigned long long) 0);
        case 12:
            return __builtin_classify_type((long *) 0);
        case 13:
            return __builtin_classify_type((enum Enum1) 0);
        case 14:
            return __builtin_classify_type((long[]){0});
        case 15:
            return __builtin_classify_type(abort);
        case 16:
            return __builtin_classify_type((struct Struct1){0});
        case 17:
            return __builtin_classify_type((union Union1){0});
        default:
            return -1;
    }
}

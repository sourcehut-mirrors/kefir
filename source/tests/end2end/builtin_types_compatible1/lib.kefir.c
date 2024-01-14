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

enum Enum1 { X = 1 };

enum Enum2 { Y = 2 };

struct Struct1 {
    int x;
    int y;
};

struct Struct2 {
    int x;
    int y;
};

int arr[] = {__builtin_types_compatible_p(const char, char),
             __builtin_types_compatible_p(char, int),
             __builtin_types_compatible_p(float, volatile float),
             __builtin_types_compatible_p(long, float),
             __builtin_types_compatible_p(short *, short *volatile const),
             __builtin_types_compatible_p(int *, int *const *),
             __builtin_types_compatible_p(int[], int[100]),
             __builtin_types_compatible_p(int[], int[][]),
             __builtin_types_compatible_p(enum Enum1, enum Enum1),
             __builtin_types_compatible_p(enum Enum2, enum Enum1),
             __builtin_types_compatible_p(struct Struct1, struct Struct1),
             __builtin_types_compatible_p(struct Struct1, struct Struct2),
             __builtin_types_compatible_p(void (*)(int, int *), void (*)(int, int *)),
             __builtin_types_compatible_p(void (*)(int, int *), void (*)(int))};

int arr_len = sizeof(arr) / sizeof(arr[0]);

int test(int x) {
    switch (x) {
        case 0:
            return __builtin_types_compatible_p(const char, char);

        case 1:
            return __builtin_types_compatible_p(char, int);

        case 2:
            return __builtin_types_compatible_p(float, volatile float);

        case 3:
            return __builtin_types_compatible_p(long, float);

        case 4:
            return __builtin_types_compatible_p(short *, short *volatile const);

        case 5:
            return __builtin_types_compatible_p(int *, int *const *);

        case 6:
            return __builtin_types_compatible_p(int[], int[100]);

        case 7:
            return __builtin_types_compatible_p(int[], int[][]);

        case 8:
            return __builtin_types_compatible_p(enum Enum1, enum Enum1);

        case 9:
            return __builtin_types_compatible_p(enum Enum2, enum Enum1);

        case 10:
            return __builtin_types_compatible_p(struct Struct1, struct Struct1);

        case 11:
            return __builtin_types_compatible_p(struct Struct1, struct Struct2);

        case 12:
            return __builtin_types_compatible_p(void (*)(int, int *), void (*)(int, int *));

        case 13:
            return __builtin_types_compatible_p(void (*)(int, int *), void (*)(int));

        default:
            return -1;
    }
}

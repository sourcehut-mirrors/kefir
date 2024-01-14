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

extern void do_assert(int);

struct X {
    int a;
    char b;
    double c;
    short s[10];
};

long test_compound_literals(long arg) {
    long *arr1 = (long[]){arg, arg / 2, arg + 1};
    long *arr2 = (long[]){-arg / 3};
    long *arr3 = (long[]){arg};

    int *test1, *test2;
    void *test3;
    {
        test1 = (int[]){1, 2, 3};
        test2 = (int[]){4, 5, 6};
        do_assert(test1 != test2 && test1 != (void *) arr1 && test1 != (void *) arr2 && test1 != (void *) arr3 &&
                  test2 != (void *) arr1 && test2 != (void *) arr2 && test2 != (void *) arr3);
        do_assert(test1[0] == 1 && test1[1] == 2 && test1[2] == 3);
        do_assert(test2[0] == 4 && test2[1] == 5 && test2[2] == 6);
        {
            test3 = &(struct X){0};
            do_assert(test1 != test3 && test2 != test3);
            {
                (void) (struct X){1, 2, 3, 4, 5};
                (void) (float[]){3.14159f, 2.718281828f};
                do_assert(test1[0] == 1 && test1[1] == 2 && test1[2] == 3);
                do_assert(test2[0] == 4 && test2[1] == 5 && test2[2] == 6);
            }
        }
    }
    {
        (void) (struct X){1, 2, 3, 4, 5};
        (void) (float[]){3.14159f, 2.718281828f};
    }

    do_assert(arr1 != arr2 && arr1 != arr3 && arr2 != arr3);
    return arr1[0] + arr1[1] + arr1[2] + arr2[0] + arr3[0];
}

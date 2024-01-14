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

#include "definitions.h"

void printf(const char *, ...);

int test(void) {
#define ASSERT(x) \
    if (!(x))     \
        return -1;
    void *ptr1, *ptr2, *ptr3;
    ptr1 = &(int){0};
    long x = 10;
    int y = 1000;
    short z = 50;
    long result;
    ASSERT(!((void *) &x == (void *) &y || (void *) &x == (void *) &z || (void *) &x == (void *) &result ||
             (void *) &y == (void *) &z || (void *) &y == (void *) &result || (void *) &z == (void *) &result));
    {
        int z = 80;
        result = x + y - z;
        ptr2 = &(long){1};
    }
    {
        int result = 0;
        {
            ptr3 = &(short){2};
            result++;
        }
    }
    {
        result += (int){800};
        { result -= (long){5} + (short){100}; }
    }
    ASSERT(ptr1 != ptr2 && ptr1 != ptr3 && ptr2 != ptr3);

    {
        int test;
        ptr1 = &test;
    }
    {
        int test2;
        ptr2 = &test2;
    }
    ASSERT(ptr1 != ptr2);

    return result;
}
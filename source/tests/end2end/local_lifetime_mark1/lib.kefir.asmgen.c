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

int test1(int a, int b, volatile int c) {
    return a * b + c;
}

int test2(int x) {
    volatile int a = 0;
    volatile double b = 1.0;
    volatile struct {
        int a;
        int b;
    } c = {1, 2};
    volatile char d[16];
    volatile short e[x];
    {
        volatile int f = 16;
        if (a) {
            volatile int g = 123;
        }
        while (d[0]) {
            b += 1.0;
        }
    }
    do {
        a += e[9];
    } while (e[10]);
    for (;;) {
        volatile long y = 10;
        goto label1;
    }
    {
    label1:
        test2(a);
        goto label2;
        {
            goto label3;
            volatile int z;
        label2:
            goto label1;
        }
    label3:
        test2(c.a);
    }
    {
        volatile long arr2[x];
        {
            {
                volatile int arr1[x];
                goto label4;
            }
        }
        {
        label4:
            test2(x);
        }
        volatile float arr3[x];
    }
}

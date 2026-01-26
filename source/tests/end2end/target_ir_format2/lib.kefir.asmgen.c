/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

int test1(char x, char y) {
    return x / y;
}

int test2(_Atomic int *x, int y) {
    return *x += y;
}

__int128 test3(__int128 x, __int128 y) {
    return x + y;
}

int test4(int x) {
    static void *arr[] = {&&label1, &&label2};
label1:
    goto *arr[x];
label2:
    return 10;
}

extern void init_array() {
    static int array[10];
    static int x;
    asm("lea %1, %rax\n"
        "mov %rax, (%0)\n"
        "lea %3, %rax\n"
        "mov %rax, (%2)\n"
        "lea %5, %rax\n"
        "mov %rax, (%4)\n"
        "lea %7, %rax\n"
        "mov %rax, (%6)\n"
        "mov %9, %rax\n"
        "mov %rax, (%8)"
        :
        : "i"(&array[0]), "i"(test4), "i"(&array[1]), "i"(&array[5]), "i"(&array[2]), "i"("Hello" + 2), "i"(&array[3]),
          "i"(&x), "i"(&array[4]), "n"(4.21)
        : "rax", "rbx");
}
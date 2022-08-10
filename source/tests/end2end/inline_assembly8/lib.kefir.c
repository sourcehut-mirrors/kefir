/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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

#ifdef __x86_64__
static int x;

extern int *getx() {
    return &x;
}

extern void init_array() {
    asm("lea %rax, %1\n"
        "mov [%0], %rax\n"
        "lea %rax, %3\n"
        "mov [%2], %rax\n"
        "lea %rax, %5\n"
        "mov [%4], %rax\n"
        "lea %rax, %7\n"
        "mov [%6], %rax\n"
        "mov %rax, %9\n"
        "mov [%8], %rax"
        :
        : "i"(&array[0]), "i"(fn1), "i"(&array[1]), "i"(&array[5]), "i"(&array[2]), "i"("Hello" + 2), "i"(&array[3]),
          "i"(&x), "i"(&array[4]), "i"(4.21)
        : "rax");
}
#endif

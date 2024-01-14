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

#ifdef __x86_64__
static int x;

extern int *getx() {
    return &x;
}

extern void init_array() {
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
        : "i"(&array[0]), "i"(fn1), "i"(&array[1]), "i"(&array[5]), "i"(&array[2]), "i"("Hello" + 2), "i"(&array[3]),
          "i"(&x), "i"(&array[4]), "n"(4.21)
        : "rax", "rbx");
}
#endif

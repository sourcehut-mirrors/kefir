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
long factorial(long x) {
    long result = 1;
begin:
    asm("cmpq %[one], %[x]\n"
        "jle %l3\n"
        "imul %[x], %[result]\n"
        "decq %[x]\n"
        "jmp %l[begin]"
        : [x] "+m"(x), [result] "+r"(result)
        : [one] "i"(1)
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "cc"
        : begin, end);
end:
    return result;
}
#endif

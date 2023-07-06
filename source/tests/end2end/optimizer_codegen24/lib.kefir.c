/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
asm(".global custom_hypot\n"
    "custom_hypot:\n"
    "   mulsd %xmm0, %xmm0\n"
    "   mulsd %xmm1, %xmm1\n"
    "   addsd %xmm1, %xmm0\n"
    "   ret");

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

long test() {
label1:
label2:
label3:
label4:
label5:
label6:
label7:
label8:
label9:
label10:;
    long x;
    asm goto("movq %l10, %0"
             : "=r"(x)
             : "r"(0)
             :
             : label1, label2, label3, label4, label5, label6, label7, label8, label9, label10);
    return x;
}

#endif

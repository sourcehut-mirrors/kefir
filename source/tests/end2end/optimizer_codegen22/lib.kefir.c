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

#ifdef __x86_64__
double custom_hypot(double x, double y) {
    double result;
    asm("movq %1, %xmm0\n"
        "movq %[y], %xmm1\n"
        "mulsd %xmm0, %xmm0\n"
        "mulsd %xmm1, %xmm1\n"
        "addsd %xmm1, %xmm0\n"
        "movq %xmm0, %0"
        : "=rm"(result)
        : [x] "r"(x), [y] "m"(y)
        : "xmm0", "xmm1");
    return result;
}

enum { FIELD3 = 300 };

struct S1 init_s1() {
    struct S1 s1;
    asm("lea %0, %rbx\n"
        "movq %[field1], (%rbx)\n"
        "movl %[field2], 8(%rbx)\n"
        "movw %[field3], 12(%rbx)\n"
        "movb %[field4], 14(%rbx)"
        : "=m"(s1)
        : [field1] "i"(100), [field2] "n"(FIELD3 / 3 * 2), [field3] "i"(FIELD3), [field4] "n"('X')
        : "rbx");
    return s1;
}

long clear8(long x) {
    asm("movb $0, %b0" : "+r"(x));
    return x;
}

long clear16(long x) {
    asm("movw $0, %w0" : "+r"(x));
    return x;
}

long clear32(long x) {
    asm("movl $0, %d0" : "=r"(x));
    return x;
}

long clear64(long x) {
    asm("movq $0, %q0" : "=r"(x));
    return x;
}

long set8(long x) {
    asm("xor %al, %al\n"
        "not %al\n"
        "movb %al, %b0"
        : "=m"(x)
        :
        : "al");
    return x;
}

long set16(long x) {
    asm("xor %ax, %ax\n"
        "not %ax\n"
        "movw %ax, %w0"
        : "=m"(x)
        :
        : "ax");
    return x;
}

long set32(long x) {
    asm("xor %eax, %eax\n"
        "not %eax\n"
        "movl %eax, %d0"
        : "=m"(x)
        :
        : "eax");
    return x;
}

long set64(long x) {
    asm("xor %rax, %rax\n"
        "not %rax\n"
        "movq %rax, %q0"
        : "=m"(x)
        :
        : "rax");
    return x;
}
long sum3_one(long a, long b, long c) {
    long result;
    asm("add %[arg1], %[result]\n"
        "add %[arg2], %[arg3]\n"
        "add %[arg3], %[result]"
        : [result] "=r"(result), [arg3] "=r"(c)
        : [arg1] "r"(a), [arg2] "r"(b), "1"(c), [imm] "0"(1)
        : "rax");
    return result;
}

struct S2 make_s2(unsigned int i) {
    struct S2 res;
    asm("" : "=r"(res) : "0"(i));
    return res;
}

unsigned int unwrap_s2(struct S2 s) {
    unsigned int res;
    asm("" : "=r"(res) : "0"(s));
    return res;
}

unsigned int cast_int(unsigned long l) {
    unsigned int res;
    asm("" : "=r"(res) : "0"(l));
    return res;
}

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

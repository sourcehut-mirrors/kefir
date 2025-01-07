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
long clear8(long x) {
    asm("movb $0, %b0" : "=r"(x));
    return x;
}

long clear16(long x) {
    asm("movw $0, %w0" : "=r"(x));
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
#endif

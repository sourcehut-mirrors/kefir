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
long test1(long a, long b, long c, long d) {
    long result;
    asm("add %[a], %[b]\n"
        "add %[b], %[c]\n"
        "xor %[c], %[d]\n"
        "mov %[d], %0"
        : "=m"(result)
        : [a] "m"(a), [b] "r"(b), [c] "m"(c), [d] "r"(d));
    return result;
}

int sum1(struct S1 s) {
    int result;
    asm("mov %d1, %[result]\n"
        "mov $32, %cl\n"
        "shr %cl, %1\n"
        "add %d1, %[result]"
        : [result] "=r"(result)
        : "r"(s)
        : "cl");
    return result;
}

long sum2(struct S2 s) {
    long result = 0;
    asm("add %[arg], %[result]\n"
        "add 8%[arg], %[result]\n"
        "add 16%[arg], %[result]\n"
        "add 24%[arg], %[result]\n"
        "add 32%[arg], %[result]\n"
        "add 40%[arg], %[result]"
        : [result] "+r"(result)
        : [arg] "m"(s));
    return result;
}
#endif

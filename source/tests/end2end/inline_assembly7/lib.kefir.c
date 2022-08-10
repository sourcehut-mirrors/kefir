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
long sum3_one(long a, long b, long c) {
    long result;
    asm("add %[result], %[arg1]\n"
        "add %[arg3], %[arg2]\n"
        "add %[result], %[arg3]"
        : [result] "=r"(result), [arg3] "=r"(c)
        : [arg1] "r"(a), [arg2] "r"(b), "1"(c), [imm] "0"(1)
        : "rax");
    return result;
}

struct S1 make_s1(unsigned int i) {
    struct S1 res;
    asm("" : "=r"(res) : "0"(i));
    return res;
}

unsigned int unwrap_s1(struct S1 s) {
    unsigned int res;
    asm("" : "=r"(res) : "0"(s));
    return res;
}

unsigned int cast_int(unsigned long l) {
    unsigned int res;
    asm("" : "=r"(res) : "0"(l));
    return res;
}
#endif

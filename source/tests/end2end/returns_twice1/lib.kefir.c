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

#if !defined(__NetBSD__)
struct my_jmp_buf {
    char buf[512];
};

static struct my_jmp_buf buf = {0};
extern int setjmp(struct my_jmp_buf *) __attribute__((returns_twice));
extern _Noreturn void longjmp(struct my_jmp_buf *, int);

long test1(long a, long b, long c, long d, long e, long f) {
    if (setjmp(&buf) == 0) {
        int t1 = gen1(1);
        int t2 = gen1(1);
        int t3 = gen1(1);
        int t4 = gen1(1);
        int t5 = gen1(1);
        int t6 = gen1(1);
        int t7 = gen1(1);
        longjmp(&buf, t1 + t2 + t3 + t4 + t5 + t6 + t7);
    } else {
        return a + b + c + d + e + f;
    }
}
#endif

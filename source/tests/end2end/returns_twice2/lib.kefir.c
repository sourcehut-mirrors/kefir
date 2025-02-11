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

#if !defined(__NetBSD__)
struct my_jmp_buf {
    char buf[512];
};

static struct my_jmp_buf buf = {0};
extern int setjmp(struct my_jmp_buf *) __attribute__((returns_twice));
extern _Noreturn void longjmp(struct my_jmp_buf *, int);

void test3(void) {
    void *x = test1();
    setjmp(&buf);
    test2(x);
}
#endif

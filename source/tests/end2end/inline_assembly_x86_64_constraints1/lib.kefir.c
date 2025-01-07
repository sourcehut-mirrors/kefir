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
long test_syscall(long n, long p1, long p2, long p3, long p4, long p5, long p6) {
    long result;
    register long r10 asm("r10") = p4;
    register long r8 asm("r8") = p5;
    register long r9 asm("r9") = p6;
    asm("call syscall_emu"
        : "=a"(result)
        : "a"(n), "D"(p1), "S"(p2), "d"(p3), "r"(r10), "r"(r8), "r"(r9)
        : "rcx", "r11", "memory");
    return result;
}
#endif

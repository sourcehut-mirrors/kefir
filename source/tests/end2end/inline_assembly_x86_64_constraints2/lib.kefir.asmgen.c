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

void test1(long n, double m) {
    asm volatile("# %0, %1, %2, %3, %4, %5, %6, %7, %8, %9, %10"
                 :
                 : "X"(n), "X"(m), "X"(n + 1), "X"(m + 1), "X"(n + m), "X"(n * m), "X"(n / m), "X"(-n - m), "X"(-n / m),
                   "X"(n++), "X"(m++)
                 : "rax", "rbx", "rcx", "rdx", "r11", "r13", "xmm0", "xmm3", "xmm6", "xmm10", "xmm13", "xmm15", "xmm7");
}

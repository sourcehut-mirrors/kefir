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

#ifdef __x86_64__
extern double discriminant(double a, double b, double c) {
    register double arg1 asm("xmm1") = a;
    register double arg2 asm("xmm4") = b;
    register double arg3 asm("xmm7") = c;
    register double result asm("xmm11");

    asm("mulsd %[result], %[result]\n"
        "movdqu %[a], %xmm0\n"
        "mulsd %[c], %xmm0\n"
        "addsd %xmm0, %xmm0\n"
        "addsd %xmm0, %xmm0\n"
        "subsd %xmm0, %[result]"
        : [result] "=x"(result)
        : "0"(b), [a] "x"(arg1), [c] "x"(arg3)
        : "xmm0");
    return result;
}
#endif

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
double custom_hypot(double x, double y) {
    double result;
    asm("movd %1, %xmm0\n"
        "movd %[y], %xmm1\n"
        "mulsd %xmm0, %xmm0\n"
        "mulsd %xmm1, %xmm1\n"
        "addsd %xmm1, %xmm0\n"
        "movd %xmm0, %0"
        : "=rm"(result)
        : [x] "r"(x), [y] "m"(y)
        : "xmm0", "xmm1");
    return result;
}
#endif

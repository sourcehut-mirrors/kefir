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
extern double sumall(double args[static 10]) {
    double result;
    asm("addsd %2, %0\n"
        "addsd %3, %0\n"
        "addsd %4, %0\n"
        "addsd %5, %0\n"
        "addsd %6, %0\n"
        "addsd %7, %0\n"
        "addsd %8, %0\n"
        "addsd %9, %0\n"
        "addsd %10, %0\n"
        "addsd %11, %0\n"
        : "=x"(result)
        : "0"(0.0), "xm"(args[0]), "xm"(args[1]), "xm"(args[2]), "xm"(args[3]), "xm"(args[4]), "xm"(args[5]),
          "xm"(args[6]), "xm"(args[7]), "xm"(args[8]), "xm"(args[9])
        : "xmm5", "xmm9", "xmm3", "xmm11", "xmm14", "xmm4");
    return result;
}
#endif

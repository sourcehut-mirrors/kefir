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

_Complex float test32(_Complex float x) {
    asm("movq %0, %xmm0\n"
        "shufps $0xe1, %xmm0, %xmm0\n"
        "movq %xmm0, %0"
        : "+rm"(x)
        :
        : "xmm0");
    return x;
}

_Complex double test64(_Complex double x) {
    asm("movdqu %0, %xmm0\n"
        "shufpd $0x1, %xmm0, %xmm0\n"
        "movdqu %xmm0, %0"
        : "+rm"(x)
        :
        : "xmm0");
    return x;
}

_Complex long double testld(_Complex long double x) {
    asm("lea %0, %rax\n"
        "fldt (%rax)\n"
        "fldt 16(%rax)\n"
        "fstpt (%rax)\n"
        "fstpt 16(%rax)"
        : "+rm"(x)
        :
        : "rax", "xmm0");
    return x;
}

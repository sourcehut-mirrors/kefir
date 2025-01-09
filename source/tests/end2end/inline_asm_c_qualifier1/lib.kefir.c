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

void init(struct S1 *ptr) {
#ifdef __x86_64__
    asm("movl $1, %c1(%0)\n"
        "movl $2, %c2(%0)\n"
        "movl $3, %c3(%0)\n"
        "movl $4, %c4(%0)\n"
        :
        : "r"(ptr), "n"(__builtin_offsetof(struct S1, a)), "n"(__builtin_offsetof(struct S1, c)),
          "n"(__builtin_offsetof(struct S1, f)), "n"(__builtin_offsetof(struct S1, i)));
#endif
}

/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

enum { FIELD3 = 300 };

#ifdef __x86_64__
struct S1 init_s1() {
    struct S1 s1;
    asm("lea %0, %rbx\n"
        "movq %[field1], (%rbx)\n"
        "movl %[field2], 8(%rbx)\n"
        "movw %[field3], 12(%rbx)\n"
        "movb %[field4], 14(%rbx)"
        : "=m"(s1)
        : [field1] "i"(100), [field2] "n"(FIELD3 / 3 * 2), [field3] "i"(FIELD3), [field4] "n"('X')
        : "rbx");
    return s1;
}
#endif

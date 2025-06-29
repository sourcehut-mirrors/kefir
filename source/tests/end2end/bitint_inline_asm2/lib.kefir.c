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

#ifdef __x86_64__

_BitInt(6) test1(_BitInt(6) x) {
    _BitInt(6) r;
    asm("mov %1, %%al\n"
        "mov %%al, %0\n"
        "addb $1, %0"
        : "=m"(r)
        : "m"(x)
        : "rax");
    return r;
}

_BitInt(14) test2(_BitInt(14) x) {
    _BitInt(14) r;
    asm("mov %1, %%ax\n"
        "mov %%ax, %0\n"
        "addw $1, %0"
        : "=m"(r)
        : "m"(x)
        : "rax");
    return r;
}

_BitInt(29) test3(_BitInt(29) x) {
    _BitInt(29) r;
    asm("mov %1, %%eax\n"
        "mov %%eax, %0\n"
        "addl $1, %0"
        : "=m"(r)
        : "m"(x)
        : "rax");
    return r;
}

_BitInt(60) test4(_BitInt(60) x) {
    _BitInt(60) r;
    asm("mov %1, %%rax\n"
        "mov %%rax, %0\n"
        "addq $1, %0"
        : "=m"(r)
        : "m"(x)
        : "rax");
    return r;
}

_BitInt(120) test5(_BitInt(120) x) {
    _BitInt(120) r;
    asm("mov %1, %%rax\n"
        "mov %%rax, %0\n"
        "mov 8%1, %%rax\n"
        "mov %%rax, 8%0\n"
        "addq $1, %0\n"
        "adcq $0, 8%0\n"
        : "=m"(r)
        : "m"(x)
        : "rax");
    return r;
}

#endif

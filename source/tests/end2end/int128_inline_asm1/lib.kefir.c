/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
__int128 test1(__int128 x) {
    __int128 r;
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

__int128 test2(__int128 x) {
    __int128 r;
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

__int128 test3(__int128 x) {
    __int128 r = x;
    asm("addq $1, %0\n"
        "adcq $0, 8%0\n"
        : "+m"(r));
    return r;
}
#endif

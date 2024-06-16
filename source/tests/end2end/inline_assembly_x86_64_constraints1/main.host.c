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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./definitions.h"

struct {
    long n;
    long parameters[6];
} syscall_emu_data = {0};

long syscall_emu_impl(void) {
    return (long) &syscall_emu_data;
}

#ifdef __x86_64__
__asm__(".global syscall_emu\n"
        "syscall_emu:\n"
        "mov %rax, syscall_emu_data(%rip)\n"
        "mov %rdi, syscall_emu_data+8(%rip)\n"
        "mov %rsi, syscall_emu_data+16(%rip)\n"
        "mov %rdx, syscall_emu_data+24(%rip)\n"
        "mov %r10, syscall_emu_data+32(%rip)\n"
        "mov %r8, syscall_emu_data+40(%rip)\n"
        "mov %r9, syscall_emu_data+48(%rip)\n"
        "jmp syscall_emu_impl");
#endif

int main(void) {
#ifdef __x86_64__
    long res = test_syscall(0xbad, 0xbabe, 0xcafe, 0xc0ffe, 4, 5, 6);
    assert(res == (long) &syscall_emu_data);
    assert(syscall_emu_data.n == 0xbad);
    assert(syscall_emu_data.parameters[0] == 0xbabe);
    assert(syscall_emu_data.parameters[1] == 0xcafe);
    assert(syscall_emu_data.parameters[2] == 0xc0ffe);
    assert(syscall_emu_data.parameters[3] == 4);
    assert(syscall_emu_data.parameters[4] == 5);
    assert(syscall_emu_data.parameters[5] == 6);
#endif
    return EXIT_SUCCESS;
}

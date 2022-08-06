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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "./definitions.h"

#ifdef __x86_64__
extern struct S1 init_s1();
#endif

int main() {
#ifdef __x86_64__
    struct S1 s1 = init_s1();
    assert(s1.l == 100);
    assert(s1.i == 200);
    assert(s1.s == 300);
    assert(s1.c == 'X');
#endif
    return EXIT_SUCCESS;
}

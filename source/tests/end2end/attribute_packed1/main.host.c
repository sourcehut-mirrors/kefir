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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stddef.h>
#include "./definitions.h"

int main(void) {
    struct S1 s1 = {0};
    assert(descriptor[0] == sizeof(struct S1));
    assert(descriptor[1] == _Alignof(struct S1));
    assert(descriptor[2] == offsetof(struct S1, a));
    assert(descriptor[3] == offsetof(struct S1, b));
    assert(descriptor[4] == offsetof(struct S1, c));
    assert(descriptor[5] == offsetof(struct S1, g));
    assert(descriptor[6] == offsetof(struct S1, h));
    assert(descriptor[7] == offsetof(struct S1, i));
    assert(descriptor[8] == offsetof(struct S1, j));
    assert(descriptor[9] == offsetof(struct S1, k));
    assert(descriptor[10] == offsetof(struct S1, l));
    assert(descriptor[11] == offsetof(struct S1, m));
    assert(descriptor[12] == (unsigned long) (&s1.m.a) - (unsigned long) &s1);
    assert(descriptor[13] == (unsigned long) (&s1.m.b) - (unsigned long) &s1);
    assert(descriptor[14] == (unsigned long) (&s1.m.c) - (unsigned long) &s1);
    assert(descriptor[15] == offsetof(struct S1, n));
    assert(descriptor[16] == offsetof(struct S1, o));
    assert(descriptor[17] == offsetof(struct S1, p));
    assert(descriptor[18] == offsetof(struct S1, q));
    return EXIT_SUCCESS;
}

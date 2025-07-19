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
#include <math.h>
#include <string.h>
#include "./definitions.h"

int main(void) {
    assert(s1_1.a == -5492);
    assert(fabs(s1_1.b - 2.71828f) < 1e-5);
    assert(strcmp(s1_1.c, "Hey ho!") == 0);
    assert(s1_1.d == 0xface0);
    assert(strcmp(s1_1.e, "Nope!") == 0);

    assert(s1_2.a == 10281);
    assert(fabs(s1_2.b + 3.14159f) < 1e-5);
    assert(strncmp(s1_2.c, "Goodbye, world", 10) == 0);
    assert(s1_2.d == -0xfefe);
    assert(strcmp(s1_2.e, "What?") == 0);

    assert(s2.a == -123);
    assert(s2.d.b.a == -5492);
    assert(fabs(s2.d.b.b - 2.71828f) < 1e-5);
    assert(strcmp(s2.d.b.c, "Hey ho!") == 0);
    assert(s2.d.b.d == 0xface0);
    assert(strcmp(s2.d.b.e, "Nope!") == 0);
    assert(s2.e.a == 10281);
    assert(fabs(s2.e.b + 3.14159f) < 1e-5);
    assert(strncmp(s2.e.c, "Goodbye, world", 10) == 0);
    assert(s2.e.d == -0xfefe);
    assert(strcmp(s2.e.e, "What?") == 0);

    assert(s3_1.b.a == -123);
    assert(s3_1.b.d.b.a == -5492);
    assert(fabs(s3_1.b.d.b.b - 2.71828f) < 1e-5);
    assert(strcmp(s3_1.b.d.b.c, "Hey ho!") == 0);
    assert(s3_1.b.d.b.d == 0xface0);
    assert(strcmp(s3_1.b.d.b.e, "Nope!") == 0);
    assert(s3_1.b.e.a == 10281);
    assert(fabs(s3_1.b.e.b + 3.14159f) < 1e-5);
    assert(strncmp(s3_1.b.e.c, "Goodbye, world", 10) == 0);
    assert(s3_1.b.e.d == -0xfefe);
    assert(strcmp(s3_1.b.e.e, "What?") == 0);

    assert(strncmp(s3_2.a, "Hello, world", 10) == 0);
    return EXIT_SUCCESS;
}

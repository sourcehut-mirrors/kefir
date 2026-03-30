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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "./definitions.h"

int main(void) {
    struct S3 s3;
    init(&s3);

    assert(s3.s1.a == 1000);
    assert(s3.s1.b.c == 123);
    assert(s3.s1.d.a == 2000);
    assert(s3.s1.d.b == 5678);
    assert(s3.s1.d.c == 45);
    assert(s3.s1.e.a == 1);
    assert(s3.s1.e.b == -20);
    assert(s3.s1.e.c == 30);
    assert(s3.s1.f.a == -123);
    assert(s3.s1.f.b == 4521);
    assert(s3.s1.f.c == 119);
    assert(s3.s2.a == -200);
    assert(s3.s2.b == 300);
    assert(s3.s2.c == 10);
    return EXIT_SUCCESS;
}

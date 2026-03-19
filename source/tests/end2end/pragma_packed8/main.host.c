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
#include <stddef.h>
#include "./definitions.h"

int main(void) {
#if defined(__GNUC__) || defined(__clang__) || defined(__KEFIRCC__)
    assert(s2.f0 == 1);
    assert(s2.f1.f0 == 2);
    assert(s2.f1.f1 == 3);
    assert(s2.f1.f2 == 4);
    assert(s2.f1.f3 == 5);
    assert(s2.f1.f4 == 6);
    assert(s2.f1.f5 == 7);
    assert(s2.f1.f6 == 8);
    assert(s2.f1.f7 == 9);
    assert(s2.f2 == 10);
    assert(s2.f3 == 11);
    assert(s2.f4 == 12);
    assert(s2.f5 == 13);
    assert(s2.f6 == 14);
    assert(s2.f7 == 15);
    assert(s2.f8 == 16);
    assert(s2.f9.f0 == 17);
    assert(s2.f9.f1 == 18);
    assert(s2.f9.f2 == 19);
    assert(s2.f9.f3 == 20);
    assert(s2.f9.f4 == 21);
    assert(s2.f9.f5 == 22);
    assert(s2.f9.f6 == 23);
    assert(s2.f9.f7 == 24);

    assert(s3.f0 == 1);
    assert(s3.f1.f0 == 2);
    assert(s3.f1.f1 == 3);
    assert(s3.f1.f2 == 4);
    assert(s3.f1.f3 == 5);
    assert(s3.f1.f4 == 6);
    assert(s3.f1.f5 == 7);
    assert(s3.f1.f6 == 8);
    assert(s3.f1.f7 == 9);
    assert(s3.f2 == 10);
    assert(s3.f3 == 11);
    assert(s3.f4 == 12);
    assert(s3.f5 == 13);
    assert(s3.f6 == 14);
    assert(s3.f7 == 15);
    assert(s3.f8 == 16);
    assert(s3.f9.f0 == 17);
    assert(s3.f9.f1 == 18);
    assert(s3.f9.f2 == 19);
    assert(s3.f9.f3 == 20);
    assert(s3.f9.f4 == 21);
    assert(s3.f9.f5 == 22);
    assert(s3.f9.f6 == 23);
    assert(s3.f9.f7 == 24);

    assert(s4.f0 == 1);
    assert(s4.f1.f0 == 2);
    assert(s4.f1.f1 == 3);
    assert(s4.f1.f2 == 4);
    assert(s4.f1.f3 == 5);
    assert(s4.f1.f4 == 6);
    assert(s4.f1.f5 == 7);
    assert(s4.f1.f6 == 8);
    assert(s4.f1.f7 == 9);
    assert(s4.f2 == 10);
    assert(s4.f3 == 11);
    assert(s4.f4 == 12);
    assert(s4.f5 == 13);
    assert(s4.f6 == 14);
    assert(s4.f7 == 15);
    assert(s4.f8 == 16);
    assert(s4.f9.f0 == 17);
    assert(s4.f9.f1 == 18);
    assert(s4.f9.f2 == 19);
    assert(s4.f9.f3 == 20);
    assert(s4.f9.f4 == 21);
    assert(s4.f9.f5 == 22);
    assert(s4.f9.f6 == 23);
    assert(s4.f9.f7 == 24);

    assert(s5.f0 == 1);
    assert(s5.f1.f0 == 2);
    assert(s5.f1.f1 == 3);
    assert(s5.f1.f2 == 4);
    assert(s5.f1.f3 == 5);
    assert(s5.f1.f4 == 6);
    assert(s5.f1.f5 == 7);
    assert(s5.f1.f6 == 8);
    assert(s5.f1.f7 == 9);
    assert(s5.f2 == 10);
    assert(s5.f3 == 11);
    assert(s5.f4 == 12);
    assert(s5.f5 == 13);
    assert(s5.f6 == 14);
    assert(s5.f7 == 15);
    assert(s5.f8 == 16);
    assert(s5.f9.f0 == 17);
    assert(s5.f9.f1 == 18);
    assert(s5.f9.f2 == 19);
    assert(s5.f9.f3 == 20);
    assert(s5.f9.f4 == 21);
    assert(s5.f9.f5 == 22);
    assert(s5.f9.f6 == 23);
    assert(s5.f9.f7 == 24);

    assert(s6.f0 == 1);
    assert(s6.f1.f0 == 2);
    assert(s6.f1.f1 == 3);
    assert(s6.f1.f2 == 4);
    assert(s6.f1.f3 == 5);
    assert(s6.f1.f4 == 6);
    assert(s6.f1.f5 == 7);
    assert(s6.f1.f6 == 8);
    assert(s6.f1.f7 == 9);
    assert(s6.f2 == 10);
    assert(s6.f3 == 11);
    assert(s6.f4 == 12);
    assert(s6.f5 == 13);
    assert(s6.f6 == 14);
    assert(s6.f7 == 15);
    assert(s6.f8 == 16);
    assert(s6.f9.f0 == 17);
    assert(s6.f9.f1 == 18);
    assert(s6.f9.f2 == 19);
    assert(s6.f9.f3 == 20);
    assert(s6.f9.f4 == 21);
    assert(s6.f9.f5 == 22);
    assert(s6.f9.f6 == 23);
    assert(s6.f9.f7 == 24);
#endif
    return EXIT_SUCCESS;
}

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
#include "kefir/core/util.h"
#include "kefir/test/unit_test.h"

long xchg00(long, long, long);
long xchg01(long, long, long);
long xchg02(long, long, long);
long xchg10(long, long, long);
long xchg11(long, long, long);
long xchg12(long, long, long);
long xchg20(long, long, long);
long xchg21(long, long, long);
long xchg22(long, long, long);

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    ASSERT(xchg00(1, 2, 3) == 3);
    ASSERT(xchg01(1, 2, 3) == 2);
    ASSERT(xchg02(1, 2, 3) == 1);
    ASSERT(xchg10(1, 2, 3) == 2);
    ASSERT(xchg11(1, 2, 3) == 3);
    ASSERT(xchg12(1, 2, 3) == 1);
    ASSERT(xchg20(1, 2, 3) == 1);
    ASSERT(xchg21(1, 2, 3) == 2);
    ASSERT(xchg22(1, 2, 3) == 3);
    return EXIT_SUCCESS;
}

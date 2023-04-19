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
#include <math.h>
#include <stddef.h>
#include "./definitions.h"

int main(void) {
    assert(X1 == (unsigned short) EXPR1);
    assert(X2 == EXPR2);
    assert(X3 == (unsigned char) EXPR3);
    assert(X4 == EXPR4);
    assert(fabs(X5 - EXPR5) < 0.001f);
    assert(fabs(X6 - EXPR6) < 0.001f);
    assert(X7 == (int) EXPR7);
    assert(X8 == 100);
    assert(X9 == offsetof(struct S1, c));
    return EXIT_SUCCESS;
}

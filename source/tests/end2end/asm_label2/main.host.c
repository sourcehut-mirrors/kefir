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
#include "./definitions.h"

const char Array1[] = "Hello, world!";
int Array1Length = sizeof(Array1);
int Array1SubN = 2;

extern const char *Array1Sub1;

const char *getArray1SubN(void);

int main(void) {
    assert(Array1Sub1 == &Array1[1]);
    assert(getArray1SubN() == &Array1[2]);
    return EXIT_SUCCESS;
}

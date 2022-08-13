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
#include <string.h>
#include "./definitions.h"

extern const char *STR1;
extern const char *STR2;
extern const char *STR3;
extern const char *STR4;

int main() {
    assert(strcmp(STR1, "char") == 0);
    assert(strcmp(STR2, "__char") == 0);
    assert(strcmp(STR3, "2*f(9)") == 0);
    assert(strcmp(STR4, "1+2*10-TEST1(100)") == 0);
    return EXIT_SUCCESS;
}

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
#include <string.h>
#include "kefir/core/util.h"
#include "kefir/test/unit_test.h"

const char *literal1(int);
const char *literal2(int);
const char *literal3(int);
const char *literal4(int);

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    const char *STRINGS[] = {"Test123, one two three...", "Hello, world!", "\t\t\n\t\tHell \vwrld!\n\t\0"};
    for (unsigned int i = 0; i < strlen(STRINGS[0]); i++) {
        ASSERT(strcmp(STRINGS[0] + i, literal1(i)) == 0);
    }
    for (unsigned int i = 0; i < strlen(STRINGS[1]); i++) {
        ASSERT(strcmp(STRINGS[1] + i, literal2(i)) == 0);
    }
    ASSERT(*literal3(0) == '\0');
    for (unsigned int i = 0; i < strlen(STRINGS[2]); i++) {
        ASSERT(strcmp(STRINGS[2] + i, literal4(i)) == 0);
    }
    return EXIT_SUCCESS;
}

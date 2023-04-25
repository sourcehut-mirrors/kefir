/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

char *get_character1(void);
unsigned *get_integer1(void);
long *get_long1(void);
float *get_float1(void);
double *get_double1(void);
char *get_str1(void);
char **get_str2(void);
char **get_str3(void);
unsigned **get_int1ptr(void);
void **get_fnptr(void);
void **get_null_ptr(void);

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    ASSERT(*get_character1() == 'C');
    ASSERT(*get_integer1() == 0xfefea6);
    ASSERT(*get_long1() == -1234543);
    ASSERT(FLOAT_EQUALS(*get_float1(), 9.57463f, FLOAT_EPSILON));
    ASSERT(DOUBLE_EQUALS(*get_double1(), 1.564e14, DOUBLE_EPSILON));
    ASSERT(strcmp(get_str1(), "Ping-pong-ping-pong") == 0);
    ASSERT(strcmp(*get_str2(), "   ....\t\t\t\\\n\n\n...TEST\n ") == 0);
    ASSERT(*get_str3() == get_str1());
    ASSERT(strcmp(*get_str3(), get_str1()) == 0);
    ASSERT(*get_int1ptr() == get_integer1());
    ASSERT(**get_int1ptr() == 0xfefea6);
    unsigned *(*someptr)(void) = get_integer1;
    ASSERT(*get_fnptr() == *(void **) &someptr);
    ASSERT(*get_null_ptr() == NULL);
    return EXIT_SUCCESS;
}

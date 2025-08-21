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

static int count = 0;
int somefn(char *buf, unsigned long *len1, char *buf2, unsigned long *len2) {
    (void) buf;
    (void) buf2;
    if (*len1 > 20) {
        *len1 = *len1 - 20;
    }
    if (*len2 > 20) {
        *len2 = *len2 - 20;
    }
    if (count++ > 2) {
        abort();
    }
    return 0;
}

int main(void) {
    char buf[128], buf2[128];

    test1(0, buf, sizeof(buf), buf2, sizeof(buf2));
    assert(count == 2);
    return EXIT_SUCCESS;
}

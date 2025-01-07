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
#include <string.h>
#include "./definitions.h"

int main(void) {
    assert(t1.len == 5);
    assert(t1.content[0] == 1);
    assert(t1.content[1] == 2);
    assert(t1.content[2] == 3);
    assert(t1.content[3] == sizeof(struct Test1));
    assert(t1.content[4] == 5);

    assert(t2 != NULL);
    assert(t2->len == 6);
    assert(t2->content[0] == 10);
    assert(t2->content[1] == 9);
    assert(t2->content[2] == 8);
    assert(t2->content[3] == 7);
    assert(t2->content[4] == 6);
    assert(t2->content[5] == 5);

    struct Test1 *t3p = t3();
    assert(t3p != NULL);
    assert(t3p->len == 3);
    assert(t3p->content[0] == 4);
    assert(t3p->content[1] == 5);
    assert(t3p->content[2] == 6);

    t3p = get_T2(0);
    assert(t3p != NULL);
    assert(t3p->len == 3);
    t3p = get_T2(1);
    assert(t3p != NULL);
    assert(t3p->len == 20);
    t3p = get_T2(2);
    assert(t3p == NULL);

    assert(T3.len == strlen("HELLO WORLD") + 1);
    assert(strcmp(T3.content, "HELLO WORLD") == 0);
    return EXIT_SUCCESS;
}

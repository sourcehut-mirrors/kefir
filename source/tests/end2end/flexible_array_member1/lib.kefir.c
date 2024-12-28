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

#define INTERNAL
#include "./definitions.h"

struct Test2 {
    int len;
    struct Test1 content[];
};

struct Test1 t1 = {.len = 5, .content = {1, 2, 3, sizeof(struct Test1), 5}};

static struct Test1 t2s = {6, 10, 9, 8, 7, 6, 5};

struct Test1 *t2 = &t2s;

struct Test1 *t3(void) {
    static struct Test1 t3 = {3, {4, 5, 6}};
    return &t3;
}

static struct Test2 T1 = {.len = 2, .content = {(struct Test1){.len = 3}, {.len = 20}}};

struct Test1 *get_T2(int x) {
    if (x >= 0 && x < T1.len) {
        return &T1.content[x];
    } else {
        return (void *) 0;
    }
}

#define PHRASE "HELLO WORLD"
struct Test3 T3 = {.len = sizeof(PHRASE), PHRASE};

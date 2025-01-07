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

struct param {
    unsigned char a : 1;
    unsigned long b : 1;
    unsigned char c : 7;
};

int sum(struct param *);

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);

    for (unsigned int i = 0; i < 50; i++) {
        ASSERT(sum(&(struct param){.a = 0, .b = 0, .c = i}) ==
               (int) (i + sizeof(struct param) + _Alignof(struct param)));

        ASSERT(sum(&(struct param){.a = 1, .b = 0, .c = i}) ==
               (int) (i + 1 + sizeof(struct param) + _Alignof(struct param)));

        ASSERT(sum(&(struct param){.a = 0, .b = 1, .c = i}) ==
               (int) (i + 1 + sizeof(struct param) + _Alignof(struct param)));

        ASSERT(sum(&(struct param){.a = 1, .b = 1, .c = i}) ==
               (int) (i + 2 + sizeof(struct param) + _Alignof(struct param)));
    }
    return EXIT_SUCCESS;
}

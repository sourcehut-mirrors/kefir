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

typedef struct multivalue {
    enum { MULTIVALUE_INT, MULTIVALUE_FLOAT, MULTIVALUE_PTR } type;

    union {
        long long int64;
        double float64;
        void *ptr;
    } value;
} multivalue_t;

extern multivalue_t multi1;
extern multivalue_t multiarr[];

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    ASSERT(multi1.type == MULTIVALUE_FLOAT);
    ASSERT(FLOAT_EQUALS(multi1.value.float64, 8.163f, FLOAT_EPSILON));
    ASSERT(multiarr[0].type == MULTIVALUE_INT);
    ASSERT(multiarr[0].value.int64 == 0xffe);
    ASSERT(multiarr[1].type == MULTIVALUE_PTR);
    ASSERT(multiarr[1].value.ptr == &multi1);
    ASSERT(multiarr[2].type == MULTIVALUE_FLOAT);
    ASSERT(FLOAT_EQUALS(multiarr[2].value.float64, 6.67f, FLOAT_EPSILON));
    ASSERT(multiarr[3].type == 0);
    ASSERT(multiarr[3].value.int64 == 0);
    ASSERT(multiarr[4].type == 0);
    ASSERT(multiarr[4].value.int64 == 0);
    ASSERT(multiarr[5].type == MULTIVALUE_PTR);
    multivalue_t *multi2 = multiarr[5].value.ptr;
    ASSERT(multi2->type == MULTIVALUE_PTR);
    ASSERT(multi2->value.ptr == multiarr);
    return EXIT_SUCCESS;
}

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

#include <stdlib.h>
#include <stdio.h>
#include "kefir/core/util.h"
#include "kefir/test/unit_test.h"

double sum(double, double);
double sub(double, double);
double mul(double, double);
double divide(double, double);
int less(double, double);
int lesseq(double, double);
int greater(double, double);
int greatereq(double, double);
int equals(double, double);
int noteq(double, double);

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    for (double i = -10.0f; i < 10.0f; i += 0.1f) {
        for (double j = -10.0f; j < 10.0f; j += 0.1f) {
            ASSERT(DOUBLE_EQUALS(sum(i, j), i + j, DOUBLE_EPSILON));
            ASSERT(DOUBLE_EQUALS(sub(i, j), i - j, DOUBLE_EPSILON));
            ASSERT(DOUBLE_EQUALS(mul(i, j), i * j, DOUBLE_EPSILON));
            ASSERT(DOUBLE_EQUALS(divide(i, j), i / j, DOUBLE_EPSILON));

            ASSERT(less(i, j) == (i < j));
            ASSERT(lesseq(i, j) == (i <= j));
            ASSERT(greater(i, j) == (i > j));
            ASSERT(greatereq(i, j) == (i >= j));
            ASSERT(equals(i, j) == (i == j));
            ASSERT(noteq(i, j) == (i != j));
        }
    }
    return EXIT_SUCCESS;
}

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
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <float.h>
#include "./definitions.h"

int main(void) {
    assert(isinf(my_inff()));
    assert(isinf(my_inf()));
    assert(isinf((double) my_infl()));

    assert(isinf(my_huge_valf()) || fabs(my_huge_valf() - FLT_MAX) < 1e-3);
    assert(isinf(my_huge_val()) || fabs(my_huge_val() - DBL_MAX) < 1e-6);
    assert(isinf((double) my_huge_vall()) || fabsl(my_huge_vall() - LDBL_MAX) < 1e-6);
    return EXIT_SUCCESS;
}

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
#include <limits.h>
#include "./definitions.h"

int main(void) {
    for (int i = -1000; i < 1000; i++) {
        int r;
        long lr;
        long long llr;
        assert(!overflow_smul(i, 500, &r) && r == i * 500);
        assert(overflow_smul(i, INT_MAX / 500, &r) == (i < -500 || i > 500) && r == (int) ((long) INT_MAX / 500 * i));
        assert(overflow_smul(i, INT_MIN / 500, &r) == (i < -500 || i > 500) && r == (int) ((long) INT_MIN / 500 * i));

        assert(!overflow_smull(i, 800, &lr) && lr == i * 800);
        assert(overflow_smull(i, LONG_MAX / 400, &lr) == (i < -400 || i > 400) &&
               (i < -400 || i > 400 || lr == LONG_MAX / 400 * i));
        assert(overflow_smull(i, LONG_MIN / 400, &lr) == (i < -400 || i > 400) &&
               (i < -400 || i > 400 || lr == LONG_MIN / 400 * i));

        assert(!overflow_smulll(i, 350, &llr) && llr == i * 350);
        assert(overflow_smulll(i, LLONG_MAX / 700, &llr) == (i < -700 || i > 700) &&
               (i < -700 || i > 700 || llr == LLONG_MAX / 700 * i));
        assert(overflow_smulll(i, LLONG_MIN / 700, &llr) == (i < -700 || i > 700) &&
               (i < -700 || i > 700 || llr == LLONG_MIN / 700 * i));
    }

    for (unsigned int i = 0; i < 4096; i++) {
        unsigned int r;
        unsigned long int lr;
        unsigned long long int llr;
        assert(!overflow_umul(i, 500, &r) && r == i * 500);
        assert(overflow_umul(UINT_MAX / 1000, i, &r) == (i > 1000) && r == (unsigned int) ((long) UINT_MAX / 1000 * i));

        assert(!overflow_umull(i, 800, &lr) && lr == i * 800);
        assert(overflow_umull(ULONG_MAX / 900, i, &lr) == (i > 900) && (i > 900 || lr == ULONG_MAX / 900 * i));

        assert(!overflow_umulll(i, 1600, &llr) && llr == i * 1600);
        assert(overflow_umulll(ULLONG_MAX / 1100, i, &llr) == (i > 1100) && (i > 1100 || llr == ULONG_MAX / 1100 * i));
    }
    return EXIT_SUCCESS;
}

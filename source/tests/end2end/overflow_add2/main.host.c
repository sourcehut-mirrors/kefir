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
        assert(!overflow_sadd(i, i * 500, &r) && r == (i + i * 500));
        assert(overflow_sadd(i, INT_MAX, &r) == (i > 0) && r == (int) (i + (long) INT_MAX));
        assert(overflow_sadd(i, INT_MIN, &r) == (i < 0) && r == (int) (i + (long) INT_MIN));

        assert(!overflow_saddl(i, i * 500, &lr) && lr == (i + i * 500));
        assert(overflow_saddl(i, LONG_MAX - 300, &lr) == (i > 300) && (i > 300 || lr == i - 300 + LONG_MAX));
        assert(overflow_saddl(i, LONG_MIN + 200, &lr) == (i < -200) && (i < -200 || lr == i + 200 + LONG_MIN));

        assert(!overflow_saddll(i, i * 500, &llr) && llr == (i + i * 500));
        assert(overflow_saddll(i, LLONG_MAX - 300, &llr) == (i > 300) && (i > 300 || llr == i - 300 + LONG_MAX));
        assert(overflow_saddll(i, LLONG_MIN + 200, &llr) == (i < -200) && (i < -200 || llr == i + 200 + LLONG_MIN));
    }

    for (unsigned int i = 0; i < 4096; i++) {
        unsigned int r;
        unsigned long int lr;
        unsigned long long int llr;
        assert(!overflow_uadd(i, i * 500, &r) && r == (i + i * 500));
        assert(overflow_uadd(i, UINT_MAX - 150, &r) == (i > 150) && r == (unsigned int) (i + (long) UINT_MAX - 150));

        assert(!overflow_uaddl(i, i * 500, &lr) && lr == (i + i * 500));
        assert(overflow_uaddl(i, ULONG_MAX - 250, &lr) == (i > 250) &&
               lr == (unsigned long) (i + (unsigned long) ULONG_MAX - 250));

        assert(!overflow_uaddll(i, i * 500, &llr) && llr == (i + i * 500));
        assert(overflow_uaddll(i, ULLONG_MAX - 350, &llr) == (i > 350) &&
               llr == (unsigned long long) (i + (unsigned long) ULONG_MAX - 350));
    }
    return EXIT_SUCCESS;
}

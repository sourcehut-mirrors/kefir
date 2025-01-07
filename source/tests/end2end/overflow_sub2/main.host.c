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
        assert(!overflow_ssub(i, i * 500, &r) && r == (i - i * 500));
        assert(overflow_ssub(i, INT_MAX, &r) == (i < -1) && r == (int) (i - (long) INT_MAX));
        assert(overflow_ssub(i, INT_MIN, &r) == (i >= 0) && r == (int) (i - (long) INT_MIN));

        assert(!overflow_ssubl(i, i * 500, &lr) && lr == (i - i * 500));
        assert(overflow_ssubl(i, LONG_MAX - 300, &lr) == (i < -301) && (i < -301 || lr == i + 300 - LONG_MAX));
        assert(overflow_ssubl(i, LONG_MIN + 200, &lr) == (i >= 200) && (i >= 200 || lr == i - 200 - LONG_MIN));

        assert(!overflow_ssubll(i, i * 500, &llr) && llr == (i - i * 500));
        assert(overflow_ssubll(i, LLONG_MAX - 300, &llr) == (i < -301) && (i < -301 || llr == i + 300 - LLONG_MAX));
        assert(overflow_ssubll(i, LLONG_MIN + 200, &llr) == (i >= 200) && (i >= 200 || llr == i - 200 - LLONG_MIN));
    }

    for (unsigned int i = 0; i < 4096; i++) {
        unsigned int r;
        unsigned long int lr;
        unsigned long long int llr;
        assert(!overflow_usub(i * 500, i, &r) && r == (i * 500 - i));
        assert(overflow_usub(150, i, &r) == (i > 150) && r == (unsigned int) (150 - i));

        assert(!overflow_usubl(i * 500, i, &lr) && lr == (i * 500 - i));
        assert(overflow_usubl(250, i, &lr) == (i > 250) && lr == (unsigned long) (250ll - i));

        assert(!overflow_usubll(i * 500, i, &llr) && llr == (i * 500 - i));
        assert(overflow_usubll(350, i, &llr) == (i > 350) && llr == (unsigned long) (350ll - i));
    }
    return EXIT_SUCCESS;
}

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
#include <math.h>
#include "./definitions.h"

int main(void) {
    const float fnan = nanf("NaN");
    const double dnan = nan("NaN");
    const long double ldnan = nanl("NaN");

    assert(less_thanf(fnan, fnan) == (fnan < fnan));
    assert(less_than(dnan, dnan) == (dnan < dnan));
    assert(less_thanl(ldnan, ldnan) == (ldnan < ldnan));
    for (double x = -10.0; x < 10.0; x += 0.1) {
        for (double y = -1.0; y < 1.0; y += 0.1) {
            assert(less_thanf(x, y) == ((float) x < (float) y));
            assert(less_than(x, y) == (x < y));
            assert(less_thanl(x, y) == ((long double) x < (long double) y));
        }
    }

    assert(eq_less_thanf(fnan, fnan) == (fnan <= fnan));
    assert(eq_less_than(dnan, dnan) == (dnan <= dnan));
    assert(eq_less_thanl(ldnan, ldnan) == (ldnan <= ldnan));
    for (double x = -10.0; x < 10.0; x += 0.1) {
        for (double y = -1.0; y < 1.0; y += 0.1) {
            assert(eq_less_thanf(x, y) == ((float) x <= (float) y));
            assert(eq_less_than(x, y) == (x <= y));
            assert(eq_less_thanl(x, y) == ((long double) x <= (long double) y));
        }
    }

    assert(eqf(fnan, fnan) == (fnan == fnan));
    assert(eq(dnan, dnan) == (dnan == dnan));
    assert(eql(ldnan, ldnan) == (ldnan == ldnan));

    assert(neqf(fnan, fnan) == (fnan != fnan));
    assert(neq(dnan, dnan) == (dnan != dnan));
    assert(neql(ldnan, ldnan) == (ldnan != ldnan));

    assert(greater_thanf(fnan, fnan) == (fnan > fnan));
    assert(greater_than(dnan, dnan) == (dnan > dnan));
    assert(greater_thanl(ldnan, ldnan) == (ldnan > ldnan));
    for (double x = -10.0; x < 10.0; x += 0.1) {
        for (double y = -1.0; y < 1.0; y += 0.1) {
            assert(greater_thanf(x, y) == ((float) x > (float) y));
            assert(greater_than(x, y) == (x > y));
            assert(greater_thanl(x, y) == ((long double) x > (long double) y));
        }
    }

    assert(eq_greater_thanf(fnan, fnan) == (fnan >= fnan));
    assert(eq_greater_than(dnan, dnan) == (dnan >= dnan));
    assert(eq_greater_thanl(ldnan, ldnan) == (ldnan >= ldnan));
    for (double x = -10.0; x < 10.0; x += 0.1) {
        for (double y = -1.0; y < 1.0; y += 0.1) {
            assert(eq_greater_thanf(x, y) == ((float) x >= (float) y));
            assert(eq_greater_than(x, y) == (x >= y));
            assert(eq_greater_thanl(x, y) == ((long double) x >= (long double) y));
        }
    }
    return EXIT_SUCCESS;
}

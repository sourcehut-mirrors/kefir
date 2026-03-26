/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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
#include <complex.h>
#include "./definitions.h"

#define EPSILON_F 1e-3
#define EPSILON_D 1e-6
#define EPSILON_LD 1e-8

int main(void) {
    for (long x = -4096; x < 4096; x++) {
        _Atomic char chr;
        _Atomic short shrt;
        _Atomic int integer;
        _Atomic long lng;
#ifndef __DragonFly__
        _Atomic long double ldbl;
        _Atomic _Complex long double cldbl;
#endif

#define TEST(_prefix, _id)                                 \
    do {                                                   \
        _prefix##_atomic_store8_##_id(&chr, (char) x);     \
        assert(chr == (char) x);                           \
                                                           \
        _prefix##_atomic_store16_##_id(&shrt, (short) x);  \
        assert(shrt == (short) x);                         \
                                                           \
        _prefix##_atomic_store32_##_id(&integer, (int) x); \
        assert(integer == (int) x);                        \
                                                           \
        _prefix##_atomic_store64_##_id(&lng, (long) x);    \
        assert(lng == (long) x);                           \
    } while (0)

        TEST(test, relaxed);
        TEST(test, consume);
        TEST(test, acquire);
        TEST(test, release);
        TEST(test, acq_rel);
        TEST(test, seq_cst);

#ifndef __DragonFly__
#define TEST128(_prefix, _id)                                                  \
    do {                                                                       \
        _prefix##_atomic_store128_##_id(&ldbl, (long double) x);               \
        assert(fabsl(ldbl - (long double) x) < EPSILON_LD);                    \
                                                                               \
        _prefix##_atomic_store256_##_id(&cldbl, (_Complex long double) x + I); \
        assert(fabsl(creall(cldbl) - (long double) x) < EPSILON_LD);           \
        assert(fabsl(cimagl(cldbl) - (long double) 1) < EPSILON_LD);           \
    } while (0)

        TEST128(test, relaxed);
        TEST128(test, consume);
        TEST128(test, acquire);
        TEST128(test, release);
        TEST128(test, acq_rel);
        TEST128(test, seq_cst);
#endif
    }

    for (long x = -4096; x < 4096; x++) {
        _Atomic char chr;
        _Atomic short shrt;
        _Atomic int integer;
        _Atomic long lng;
#ifndef __DragonFly__
        _Atomic long double ldbl;
        _Atomic _Complex long double cldbl;
#endif

        TEST(test2, relaxed);
        TEST(test2, consume);
        TEST(test2, acquire);
        TEST(test2, release);
        TEST(test2, acq_rel);
        TEST(test2, seq_cst);

#ifndef __DragonFly__
        TEST128(test2, relaxed);
        TEST128(test2, consume);
        TEST128(test2, acquire);
        TEST128(test2, release);
        TEST128(test2, acq_rel);
        TEST128(test2, seq_cst);
#endif
    }
    return EXIT_SUCCESS;
}

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
#include "./definitions.h"

int main(void) {
    int i = 0;
    assert(ivalue[i].arr[0] == 10218319);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == (unsigned long) -13839138);
    assert(ivalue[i++].arr[1] == (unsigned long) -1);
    assert(ivalue[i].arr[0] == 90013919u);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == 3267759413603604127ull);
    assert(ivalue[i++].arr[1] == 54);
    assert(ivalue[i].arr[0] == (unsigned long) -3267759413603604127ll);
    assert(ivalue[i++].arr[1] == (unsigned long) -55);
    assert(ivalue[i].arr[0] == 3);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == 1031838000ull);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == 9913);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == 3);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == 1031838000ull);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == 9913);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == 0);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == 0);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == 0);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == (unsigned long) &ivalue[1]);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == (unsigned long) -13191993);
    assert(ivalue[i++].arr[1] == (unsigned long) -1);
    assert(ivalue[i].arr[0] == 0xcedf8edefac3ull);
    assert(ivalue[i++].arr[1] == 0);
#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && \
    !defined(__NetBSD__) && !defined(__DragonFly__) &&                                                               \
    (__GNUC__ >= 14 || defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)) && !defined(KEFIR_PLATFORM_DECIMAL_DPD)
    assert(ivalue[i].arr[0] == 4);
    assert(ivalue[i++].arr[1] == 0);
    assert(ivalue[i].arr[0] == (unsigned long) -19318);
    assert(ivalue[i++].arr[1] == (unsigned long) -1);
    assert(ivalue[i].arr[0] == 38193310ull);
    assert(ivalue[i++].arr[1] == 0);
#endif

    i = 0;
    assert(uvalue[i].arr[0] == 10218319);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == (unsigned long) -13839138);
    assert(uvalue[i++].arr[1] == (unsigned long) -1);
    assert(uvalue[i].arr[0] == 90013919u);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == 3267759413603604127ull);
    assert(uvalue[i++].arr[1] == 54);
    assert(uvalue[i].arr[0] == (unsigned long) -3267759413603604127ll);
    assert(uvalue[i++].arr[1] == (unsigned long) -55);
    assert(uvalue[i].arr[0] == 3);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == 1031838000ull);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == 9913);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == 3);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == 1031838000ull);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == 9913);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == 0);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == 0);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == 0);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == (unsigned long) &uvalue[1]);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == (unsigned long) -13191993);
    assert(uvalue[i++].arr[1] == (unsigned long) -1);
    assert(uvalue[i].arr[0] == 0xcedf8edefac3ull);
    assert(uvalue[i++].arr[1] == 0);
#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && \
    !defined(__NetBSD__) && !defined(__DragonFly__) &&                                                               \
    (__GNUC__ >= 14 || defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)) && !defined(KEFIR_PLATFORM_DECIMAL_DPD)
    assert(uvalue[i].arr[0] == 4);
    assert(uvalue[i++].arr[1] == 0);
    assert(uvalue[i].arr[0] == (unsigned long) 19318);
    assert(uvalue[i++].arr[1] == (unsigned long) 0);
    assert(uvalue[i].arr[0] == 38193310ull);
    assert(uvalue[i++].arr[1] == 0);
#endif
    return EXIT_SUCCESS;
}

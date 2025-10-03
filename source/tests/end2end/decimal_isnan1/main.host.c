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

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && !defined(__NetBSD__)
#pragma GCC diagnostic ignored "-Wpedantic"
#define ENABLE_DECIMAL_TEST
int dec32_isnan(_Decimal32);
int dec64_isnan(_Decimal64);
int dec128_isnan(_Decimal128);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(!dec32_isnan(3.14));
    assert(!dec32_isnan(-3.14));
    assert(!dec32_isnan(0.0));
    assert(!dec32_isnan(-0.0));
    assert(!dec32_isnan(INFINITY));
    assert(!dec32_isnan(-INFINITY));
    assert(dec32_isnan(nan("")));

    assert(!dec64_isnan(3.14));
    assert(!dec64_isnan(-3.14));
    assert(!dec64_isnan(0.0));
    assert(!dec64_isnan(-0.0));
    assert(!dec64_isnan(INFINITY));
    assert(!dec64_isnan(-INFINITY));
    assert(dec64_isnan(nan("")));

    assert(!dec128_isnan(3.14));
    assert(!dec128_isnan(-3.14));
    assert(!dec128_isnan(0.0));
    assert(!dec128_isnan(-0.0));
    assert(!dec128_isnan(INFINITY));
    assert(!dec128_isnan(-INFINITY));
    assert(dec128_isnan(nan("")));
#endif
    return EXIT_SUCCESS;
}

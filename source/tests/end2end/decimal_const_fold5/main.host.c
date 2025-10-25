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

#if ((defined(__GNUC__) && !defined(__clang__) && !defined(__KEFIRCC__)) || defined(__KEFIRCC_DECIMAL_SUPPORT__)) && !defined(__NetBSD__) && !defined(__DragonFly__) && (__GNUC__ >= 14 || defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)) && !defined(KEFIR_PLATFORM_DECIMAL_DPD)
#pragma GCC diagnostic ignored "-Wpedantic"
#define ENABLE_DECIMAL_TEST
_Bool decimal32_eq(_Decimal32 a, _Decimal32 b) {
    _Decimal32 diff = (a - b) * 10000;
    return diff < 1 && diff > -1;
}

_Bool decimal64_eq(_Decimal64 a, _Decimal64 b) {
    _Decimal64 diff = (a - b) * 100000;
    return diff < 1 && diff > -1;
}

_Bool decimal128_eq(_Decimal128 a, _Decimal128 b) {
    _Decimal128 diff = (a - b) * 1000000;
    return diff < 1 && diff > -1;
}

_Decimal32 dec32_from_signed(void);
_Decimal32 dec32_from_unsigned(void);
_Decimal64 dec64_from_signed(void);
_Decimal64 dec64_from_unsigned(void);
_Decimal128 dec128_from_signed(void);
_Decimal128 dec128_from_unsigned(void);
#endif

int main(void) {
#ifdef ENABLE_DECIMAL_TEST
    assert(decimal32_eq(dec32_from_signed(), -1e32));
    assert(decimal32_eq(dec32_from_unsigned(), 2e32));
    assert(decimal64_eq(dec64_from_signed(), -8e32));
    assert(decimal64_eq(dec64_from_unsigned(), 4e33));
    assert(decimal128_eq(dec128_from_signed(), -8e32dl));
    assert(decimal128_eq(dec128_from_unsigned(), 4e33dl));
#endif
    return EXIT_SUCCESS;
}

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

#include "definitions.h"

#define ASSERT_TYPE(_expr, _res_type) \
    _Static_assert(_Generic((_expr), _res_type: 1, default: 0), "Assertion on expression type has failed")

ASSERT_TYPE(__builtin_choose_expr(0x100000000000000000000000000000000000000000000000uwb, 1, 1.0f), int);
ASSERT_TYPE(__builtin_choose_expr(0x000000000000000000000000000000000000000000000000uwb, 1, 1.0f), float);
ASSERT_TYPE(__builtin_choose_expr(0x100000000000000000000000000000000000000000000000wb, 1, 1.0f), int);
ASSERT_TYPE(__builtin_choose_expr(-0x100000000000000000000000000000000000000000000000wb, 1, 1.0f), int);
ASSERT_TYPE(__builtin_choose_expr(0x000000000000000000000000000000000000000000000000wb, 1, 1.0f), float);
ASSERT_TYPE(__builtin_choose_expr(-0x000000000000000000000000000000000000000000000000wb, 1, 1.0f), float);

_Static_assert(0xf000000000000000000000000000000000000000000000000000000000000000000000000000000000wb, "Assert1");
_Static_assert(-0x1000000000000000000000000000000000000000000000000000000000000000000000000000000000wb, "Assert2");
_Static_assert(0x1000000000000000000000000000000000000000000000000000000000000000000000000000000000uwb, "Assert3");
_Static_assert(!0x000000000000000000000000000000000000000000000000000000000000000000000000000000000uwb, "Assert4");

#if 000000000000000000000000000000000000uwb
#error "ERROR1"
#elif 0x00000000000000wb
#error "ERROR2"
#elif 0x10000000000000000000000000000000000000000000000000000000000000wb
int get1(void) {
    return __builtin_choose_expr(0x100000000000000000000000000000000000000000000000uwb, 1, -1.0f);
}

int get2(void) {
    return __builtin_choose_expr(0x100000000000000000000000000000000000000000000000wb, 1, -1.0f);
}

int get3(void) {
    return __builtin_choose_expr(0x000000000000000000000000000000000000000000000000wb, 1, -1.0f);
}
#endif
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
#include <assert.h>
#include "./definitions.h"

int main(void) {
    assert(i8[0] == -128);
    assert(i8[1] == 0);
    assert(i16[0] == -32768);
    assert(i16[1] == 0);
    assert(i32[0] == -2147483648);
    assert(i32[1] == 0);
    assert(i64[0] == (long long) 9223372036854775808ull);
    assert(i64[1] == 0);
    assert(u8[0] == 128);
    assert(u8[1] == 0);
    assert(u16[0] == 32768);
    assert(u16[1] == 0);
    assert(u32[0] == 0);
    assert(u32[1] == 2147483648);
    assert(u64[0] == 0);
    assert(u64[1] == 9223372036854775808ull);
    return EXIT_SUCCESS;
}

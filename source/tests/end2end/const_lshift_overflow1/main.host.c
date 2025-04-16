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
#include <inttypes.h>
#include "./definitions.h"

int main(void) {
    assert(i8[0] == INT8_MIN);
    assert(i8[1] == 0);
    assert(i8[2] == 0);
    assert(i8[3] == 0);
    assert(i8[4] == 0);
    assert(i8[5] == INT8_MIN);
    assert(i16[0] == INT16_MIN);
    assert(i16[1] == 0);
    assert(i16[2] == 0);
    assert(i16[3] == 0);
    assert(i16[4] == 0);
    assert(i16[5] == INT16_MIN);
    assert(i32[0] == INT32_MIN);
    assert(i32[1] == 0);
    assert(i32[2] == 0);
    assert(i32[3] == 0);
    assert(i32[4] == 0);
    assert(i32[5] == INT32_MIN);
    assert(i64[0] == INT64_MIN);
    assert(i64[1] == 0);
    assert(i64[2] == 0);
    assert(i64[3] == 0);
    assert(i64[4] == 0);
    assert(i64[5] == INT64_MIN);

    assert(u8[0] == -INT8_MIN);
    assert(u8[1] == 0);
    assert(u8[2] == 0);
    assert(u8[3] == 0);
    assert(u8[4] == 0);
    assert(u8[5] == -INT8_MIN);
    assert(u16[0] == -INT16_MIN);
    assert(u16[1] == 0);
    assert(u16[2] == 0);
    assert(u16[3] == 0);
    assert(u16[4] == 0);
    assert(u16[5] == -INT16_MIN);
    assert(u32[0] == -(unsigned int) INT32_MIN);
    assert(u32[1] == 0);
    assert(u32[2] == 0);
    assert(u32[3] == 0);
    assert(u32[4] == 0);
    assert(u32[5] == -(unsigned int) INT32_MIN);
    assert(u64[0] == -(unsigned long) INT64_MIN);
    assert(u64[1] == 0);
    assert(u64[2] == 0);
    assert(u64[3] == 0);
    assert(u64[4] == 0);
    assert(u64[5] == -(unsigned long) INT64_MIN);
    return EXIT_SUCCESS;
}

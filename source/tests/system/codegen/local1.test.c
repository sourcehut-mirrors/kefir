/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2024  Jevgenijs Protopopovs

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
#include <string.h>
#include "kefir/core/util.h"
#include "kefir/test/unit_test.h"

struct type1 {
    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;
};

struct type1 fill(uint64_t);

int main(int argc, const char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    for (kefir_int64_t i = -100; i < 100; i++) {
        struct type1 val = fill(0xf12345678ul + i);
        ASSERT(val.u8 == ((uint8_t) 0x78) + i);
        ASSERT(val.u16 == ((uint16_t) 0x5678) + i);
        ASSERT(val.u32 == ((uint32_t) 0x12345678) + i);
        ASSERT(val.u64 == 0xf12345678ul + i);
    }
    return EXIT_SUCCESS;
}

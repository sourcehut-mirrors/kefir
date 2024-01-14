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
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include "./definitions.h"

int main(void) {
    assert(myswap16(0x0000) == 0x0000);
    assert(myswap16(0x0001) == 0x0100);
    assert(myswap16(0x0201) == 0x0102);
    assert(myswap16(0xcafe) == 0xfeca);

    assert(myswap32(0x00000000) == 0x00000000);
    assert(myswap32(0x00000001) == 0x01000000);
    assert(myswap32(0x00300001) == 0x01003000);
    assert(myswap32(0x0badbabe) == 0xbebaad0b);

    assert(myswap64(0x0000000000000001) == 0x0100000000000000);
    assert(myswap64(0x0050000000000001) == 0x0100000000005000);
    assert(myswap64(0x00500000e5000001) == 0x010000e500005000);
    assert(myswap64(0xbadc0ffedeadbabe) == 0xbebaaddefe0fdcba);
    return EXIT_SUCCESS;
}

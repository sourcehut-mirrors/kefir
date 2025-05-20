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
#include <stddef.h>
#include "./definitions.h"

int main(void) {
    assert(get(0) == 0ull);
    assert(get(1) == 0ull);
    assert(get(2) == 1ull);
    assert(get(3) == 1ull);
    assert(get(4) == 18446744073709551615ull);
    assert(get(5) == 7ull);
    assert(get(6) == 15ull);
    assert(get(7) == 18446744073709551600ull);
    assert(get(8) == 16ull);
    assert(get(9) == 32767ull);
    assert(get(10) == 32768ull);
    assert(get(11) == 32768ull);
    assert(get(12) == 65535ull);
    assert(get(13) == 65535ull);
    assert(get(14) == 9223372036854775807ull);
    assert(get(15) == 9223372036854775808ull);
    assert(get(16) == 18446744073709551615ull);
    return EXIT_SUCCESS;
}

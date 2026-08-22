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

#include "kefir/util/char32.h"
#include "kefir/core/util.h"

kefir_size_t kefir_strlen32(const kefir_char32_t *string) {
    REQUIRE(string != NULL, 0);
    kefir_size_t length = 0;
    while (string[length] != U'\0') {
        length++;
    }
    return length;
}

kefir_char32_t kefir_dectohex32(kefir_uint64_t val) {
    static kefir_char32_t DEC2HEX[] = {
        [0] = U'0',
        [1] = U'1',
        [2] = U'2',
        [3] = U'3',
        [4] = U'4',
        [5] = U'5',
        [6] = U'6',
        [7] = U'7',
        [8] = U'8',
        [9] = U'9',
        [10] = U'a',
        [11] = U'b',
        [12] = U'c',
        [13] = U'd',
        [14] = U'e',
        [15] = U'f',
    };

    return DEC2HEX[val & 0xf];
}

/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2022  Jevgenijs Protopopovs

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
#include "./definitions.h"

int main() {
    unsigned int counter = 0;
    assert(Sizes[counter++] == sizeof(__SIZE_TYPE__));
    assert(Sizes[counter++] == sizeof(__PTRDIFF_TYPE__));
    assert(Sizes[counter++] == sizeof(__WCHAR_TYPE__));
    assert(Sizes[counter++] == sizeof(__WINT_TYPE__));
    assert(Sizes[counter++] == sizeof(__INTMAX_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINTMAX_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT8_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT16_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT32_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT64_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT8_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT16_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT32_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT64_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT_LEAST8_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT_LEAST16_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT_LEAST32_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT_LEAST64_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT_LEAST8_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT_LEAST16_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT_LEAST32_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT_LEAST64_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT_FAST8_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT_FAST16_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT_FAST32_TYPE__));
    assert(Sizes[counter++] == sizeof(__INT_FAST64_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT_FAST8_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT_FAST16_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT_FAST32_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINT_FAST64_TYPE__));
    assert(Sizes[counter++] == sizeof(__INTPTR_TYPE__));
    assert(Sizes[counter++] == sizeof(__UINTPTR_TYPE__));
    return EXIT_SUCCESS;
}

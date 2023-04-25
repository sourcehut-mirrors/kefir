/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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
#include <string.h>
#include "./definitions.h"

int main(void) {
    unsigned int counter = 0;
    assert(strcmp("__SIZE_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__PTRDIFF_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__WCHAR_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__WINT_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INTMAX_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINTMAX_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT8_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT16_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT32_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT64_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT8_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT16_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT32_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT64_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT_LEAST8_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT_LEAST16_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT_LEAST32_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT_LEAST64_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT_LEAST8_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT_LEAST16_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT_LEAST32_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT_LEAST64_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT_FAST8_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT_FAST16_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT_FAST32_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INT_FAST64_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT_FAST8_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT_FAST16_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT_FAST32_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINT_FAST64_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__INTPTR_TYPE__", Types[counter++]) != 0);
    assert(strcmp("__UINTPTR_TYPE__", Types[counter++]) != 0);
    return EXIT_SUCCESS;
}

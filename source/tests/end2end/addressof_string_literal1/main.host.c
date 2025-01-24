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
#include <string.h>
#include <wchar.h>
#include "./definitions.h"

int main(void) {
    const char SAMPLE[] = "HELLO WORLD";
    const wchar_t SAMPLE2[] = L"HELLO WORLD";
    assert(strcmp(someptr, "WORLD") == 0);
    assert(wcscmp(someptr2, L"WORLD") == 0);
    for (unsigned long i = 0; i < sizeof(SAMPLE) - 1; i++) {
        assert(strcmp(get(i), &SAMPLE[i]) == 0);
        assert(wcscmp(get2(i), &SAMPLE2[i]) == 0);
    }
    return EXIT_SUCCESS;
}

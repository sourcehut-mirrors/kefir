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
#include "./definitions.h"

_Atomic struct Buffer buf;

int main(void) {
    for (long x = -4096; x < 4096; x++) {
        const struct Buffer value = {{x, ~x, x + 1, x - 1, -x, x / 2, !x, x ^ 128}};

        store_buffer(&buf, &value);

        const struct Buffer current = buf;
        assert(memcmp(&value, &current, sizeof(struct Buffer)) == 0);

        const struct Buffer loaded = load_buffer(&buf);
        assert(memcmp(&value, &loaded, sizeof(struct Buffer)) == 0);
    }
    return EXIT_SUCCESS;
}

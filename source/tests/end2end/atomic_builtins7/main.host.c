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
#include "./definitions.h"

int main(void) {
    for (long x = -4096; x < 4096; x++) {
        _Bool value = (_Bool) x;
        test_atomic_thread_fence();
        test_atomic_signal_fence();
        test_is_lock_free();

        _Bool res = test_and_set(&value);
        assert(res == (_Bool) x);
        assert(value);
        clear(&value);
        assert(!value);
    }
    return EXIT_SUCCESS;
}

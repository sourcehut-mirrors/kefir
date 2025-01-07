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
#include "./definitions.h"

int main(void) {
    for (long x = -4096; x < 4096; x++) {
        _Atomic char character = x;
        _Atomic short shrt = x;
        _Atomic int integer = x;
        _Atomic long lng = x;

        char loaded_char = test_load_char(&character);
        assert(loaded_char == character);

        short loaded_short = test_load_short(&shrt);
        assert(loaded_short == shrt);

        int loaded_int = test_load_int(&integer);
        assert(loaded_int == integer);

        int loaded_long = test_load_long(&lng);
        assert(loaded_long == loaded_long);

        test_store_char(&character, ~x);
        assert(character == (char) ~x);

        test_store_short(&shrt, ~x);
        assert(shrt == (short) ~x);

        test_store_int(&integer, ~x);
        assert(integer == (int) ~x);

        test_store_long(&lng, ~x);
        assert(lng == (long) ~x);

        assert(test_exchange_char(&character, x * 2 + 1) == (char) ~x);
        assert(character == (char) (x * 2 + 1));

        assert(test_exchange_short(&shrt, x * 2 + 1) == (short) ~x);
        assert(shrt == (short) (x * 2 + 1));

        assert(test_exchange_int(&integer, x * 2 + 1) == (int) ~x);
        assert(integer == (int) (x * 2 + 1));

        assert(test_exchange_long(&lng, x * 2 + 1) == (long) ~x);
        assert(lng == (long) (x * 2 + 1));

        char expected_char = x;
        if (expected_char != character) {
            assert(!test_compare_exchange_char(&character, &expected_char, -x));
            assert(expected_char == (char) (x * 2 + 1));
            assert(character == (char) (x * 2 + 1));
        }
        assert(test_compare_exchange_char(&character, &expected_char, -x));
        assert(character == (char) -x);

        short expected_short = x;
        if (expected_short != shrt) {
            assert(!test_compare_exchange_short(&shrt, &expected_short, -x));
            assert(expected_short == (short) (x * 2 + 1));
            assert(shrt == (short) (x * 2 + 1));
        }
        assert(test_compare_exchange_short(&shrt, &expected_short, -x));
        assert(shrt == (short) -x);

        int expected_int = x;
        if (expected_int != integer) {
            assert(!test_compare_exchange_int(&integer, &expected_int, -x));
            assert(expected_int == (int) (x * 2 + 1));
            assert(integer == (int) (x * 2 + 1));
        }
        assert(test_compare_exchange_int(&integer, &expected_int, -x));
        assert(integer == (int) -x);

        long expected_long = x;
        if (expected_long != lng) {
            assert(!test_compare_exchange_long(&lng, &expected_long, -x));
            assert(expected_long == (long) (x * 2 + 1));
            assert(lng == (long) (x * 2 + 1));
        }
        assert(test_compare_exchange_long(&lng, &expected_long, -x));
        assert(lng == (long) -x);

        assert(test_fetch_add_char(&character, 10) == (char) -x);
        assert(character == (char) (-x + 10));

        assert(test_fetch_add_short(&shrt, 10) == (short) -x);
        assert(shrt == (short) (-x + 10));

        assert(test_fetch_add_int(&integer, 10) == (int) -x);
        assert(integer == (int) (-x + 10));

        assert(test_fetch_add_long(&lng, 10) == (long) -x);
        assert(lng == (long) (-x + 10));

        assert(test_fetch_sub_char(&character, 100) == (char) (-x + 10));
        assert(character == (char) (-x - 90));

        assert(test_fetch_sub_short(&shrt, 100) == (short) (-x + 10));
        assert(shrt == (short) (-x - 90));

        assert(test_fetch_sub_int(&integer, 100) == (int) (-x + 10));
        assert(integer == (int) (-x - 90));

        assert(test_fetch_sub_long(&lng, 100) == (long) (-x + 10));
        assert(lng == (long) (-x - 90));

        assert(test_fetch_or_char(&character, (char) 0x0badbabe) == (char) (-x - 90));
        assert(character == (char) ((-x - 90) | 0x0badbabe));

        assert(test_fetch_or_short(&shrt, (short) 0x0badbabe) == (short) (-x - 90));
        assert(shrt == (short) ((-x - 90) | 0x0badbabe));

        assert(test_fetch_or_int(&integer, (int) 0x0badbabe) == (int) (-x - 90));
        assert(integer == (int) ((-x - 90) | 0x0badbabe));

        assert(test_fetch_or_long(&lng, (long) 0x0badbabe) == (long) (-x - 90));
        assert(lng == (long) ((-x - 90) | 0x0badbabe));

        assert(test_fetch_xor_char(&character, (char) 0xc0ffe) == (char) ((-x - 90) | 0x0badbabe));
        assert(character == (char) (((-x - 90) | 0x0badbabe) ^ 0xc0ffe));

        assert(test_fetch_xor_short(&shrt, (short) 0xc0ffe) == (short) ((-x - 90) | 0x0badbabe));
        assert(shrt == (short) (((-x - 90) | 0x0badbabe) ^ 0xc0ffe));

        assert(test_fetch_xor_int(&integer, (int) 0xc0ffe) == (int) ((-x - 90) | 0x0badbabe));
        assert(integer == (int) (((-x - 90) | 0x0badbabe) ^ 0xc0ffe));

        assert(test_fetch_xor_long(&lng, (long) 0xc0ffe) == (long) ((-x - 90) | 0x0badbabe));
        assert(lng == (long) (((-x - 90) | 0x0badbabe) ^ 0xc0ffe));

        assert(test_fetch_and_char(&character, (char) 0xcafe0100) == (char) (((-x - 90) | 0x0badbabe) ^ 0xc0ffe));
        assert(character == (char) ((((-x - 90) | 0x0badbabe) ^ 0xc0ffe) & 0xcafe0100));

        assert(test_fetch_and_short(&shrt, (short) 0xcafe0100) == (short) (((-x - 90) | 0x0badbabe) ^ 0xc0ffe));
        assert(shrt == (short) ((((-x - 90) | 0x0badbabe) ^ 0xc0ffe) & 0xcafe0100));

        assert(test_fetch_and_int(&integer, (int) 0xcafe0100) == (int) (((-x - 90) | 0x0badbabe) ^ 0xc0ffe));
        assert(integer == (int) ((((-x - 90) | 0x0badbabe) ^ 0xc0ffe) & 0xcafe0100));

        assert(test_fetch_and_long(&lng, (long) 0xcafe0100) == (long) (((-x - 90) | 0x0badbabe) ^ 0xc0ffe));
        assert(lng == (long) ((((-x - 90) | 0x0badbabe) ^ 0xc0ffe) & 0xcafe0100));

        test_atomic_thread_fence();
        test_atomic_signal_fence();
        test_is_lock_free();
    }
    return EXIT_SUCCESS;
}

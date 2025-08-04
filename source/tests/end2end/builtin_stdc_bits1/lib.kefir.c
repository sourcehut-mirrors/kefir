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

_Static_assert(__builtin_stdc_bit_width(1ull) == 1);
_Static_assert(__builtin_stdc_bit_width(1024ull) == 11);
_Static_assert(__builtin_stdc_bit_width(1ull << 63) == 64);

_Static_assert(__builtin_stdc_count_ones(0) == 0);
_Static_assert(__builtin_stdc_count_ones(1) == 1);
_Static_assert(__builtin_stdc_count_ones(2) == 1);
_Static_assert(__builtin_stdc_count_ones(3) == 2);
_Static_assert(__builtin_stdc_count_ones(~0u >> 1) == 8 * sizeof(int) - 1);
_Static_assert(__builtin_stdc_count_ones(~0u) == 8 * sizeof(int));
_Static_assert(__builtin_stdc_count_ones(~0ull) == 8 * sizeof(long));

_Static_assert(__builtin_stdc_count_zeros(0) == 8 * sizeof(int));
_Static_assert(__builtin_stdc_count_zeros(1) == 8 * sizeof(int) - 1);
_Static_assert(__builtin_stdc_count_zeros(1024) == 8 * sizeof(int) - 1);
_Static_assert(__builtin_stdc_count_zeros(1023) == 8 * sizeof(int) - 10);
_Static_assert(__builtin_stdc_count_zeros(~0u) == 0);
_Static_assert(__builtin_stdc_count_zeros(~0u >> 1) == 1);
_Static_assert(__builtin_stdc_count_zeros(~0ull) == 0);

_Static_assert(__builtin_stdc_first_leading_one(0) == 0);
_Static_assert(__builtin_stdc_first_leading_one(1) == 32);
_Static_assert(__builtin_stdc_first_leading_one(2) == 31);
_Static_assert(__builtin_stdc_first_leading_one(3) == 31);
_Static_assert(__builtin_stdc_first_leading_one(1 << 20) == 12);
_Static_assert(__builtin_stdc_first_leading_one(~0ull) == 1);

_Static_assert(__builtin_stdc_first_leading_zero(0) == 1);
_Static_assert(__builtin_stdc_first_leading_zero(1) == 1);
_Static_assert(__builtin_stdc_first_leading_zero(2) == 1);
_Static_assert(__builtin_stdc_first_leading_zero(3) == 1);
_Static_assert(__builtin_stdc_first_leading_zero(1 << 20) == 1);
_Static_assert(__builtin_stdc_first_leading_zero(~0ull) == 0);
_Static_assert(__builtin_stdc_first_leading_zero(~0ull >> 1) == 1);

_Static_assert(__builtin_stdc_first_trailing_one(0) == 0);
_Static_assert(__builtin_stdc_first_trailing_one(1) == 1);
_Static_assert(__builtin_stdc_first_trailing_one(2) == 2);
_Static_assert(__builtin_stdc_first_trailing_one(3) == 1);
_Static_assert(__builtin_stdc_first_trailing_one(1 << 25) == 26);
_Static_assert(__builtin_stdc_first_trailing_one(~0ull) == 1);
_Static_assert(__builtin_stdc_first_trailing_one(~0ull << 1) == 2);

_Static_assert(__builtin_stdc_first_trailing_zero(0) == 1);
_Static_assert(__builtin_stdc_first_trailing_zero(1) == 2);
_Static_assert(__builtin_stdc_first_trailing_zero(2) == 1);
_Static_assert(__builtin_stdc_first_trailing_zero(3) == 3);
_Static_assert(__builtin_stdc_first_trailing_zero(1 << 25) == 1);
_Static_assert(__builtin_stdc_first_trailing_zero(0) == 1);
_Static_assert(__builtin_stdc_first_trailing_zero(~0u) == 0);
_Static_assert(__builtin_stdc_first_trailing_zero(~0ull) == 0);

_Static_assert(!__builtin_stdc_has_single_bit(0));
_Static_assert(__builtin_stdc_has_single_bit(1));
_Static_assert(__builtin_stdc_has_single_bit(1 << 10));
_Static_assert(__builtin_stdc_has_single_bit(1 << 20));
_Static_assert(__builtin_stdc_has_single_bit(1ull << 63));
_Static_assert(!__builtin_stdc_has_single_bit((1ull << 63) | 1));
_Static_assert(!__builtin_stdc_has_single_bit(~0ull));
_Static_assert(!__builtin_stdc_has_single_bit(~0ull >> 1));
_Static_assert(__builtin_stdc_has_single_bit(~(~0ull >> 1)));

_Static_assert(__builtin_stdc_leading_ones(0) == 0);
_Static_assert(__builtin_stdc_leading_ones(1) == 0);
_Static_assert(__builtin_stdc_leading_ones(1u << (8 * sizeof(int) - 1)) == 1);
_Static_assert(__builtin_stdc_leading_ones(~0u) == (8 * sizeof(int)));
_Static_assert(__builtin_stdc_leading_ones(~3u) == (8 * sizeof(int) - 2));
_Static_assert(__builtin_stdc_leading_ones(~0ull) == (8 * sizeof(long)));
_Static_assert(__builtin_stdc_leading_ones(~3ull) == (8 * sizeof(long) - 2));

_Static_assert(__builtin_stdc_leading_zeros(0) == (8 * sizeof(int)));
_Static_assert(__builtin_stdc_leading_zeros(1) == (8 * sizeof(int) - 1));
_Static_assert(__builtin_stdc_leading_zeros(2) == (8 * sizeof(int) - 2));
_Static_assert(__builtin_stdc_leading_zeros(3) == (8 * sizeof(int) - 2));
_Static_assert(__builtin_stdc_leading_zeros(~0u) == 0);
_Static_assert(__builtin_stdc_leading_zeros(~0u << 1) == 0);
_Static_assert(__builtin_stdc_leading_zeros(~0u >> 1) == 1);
_Static_assert(__builtin_stdc_leading_zeros(~0u >> 10) == 10);
_Static_assert(__builtin_stdc_leading_zeros(~0ull) == 0);
_Static_assert(__builtin_stdc_leading_zeros(~0ull >> 3) == 3);
_Static_assert(__builtin_stdc_leading_zeros(0ull) == (8 * sizeof(long)));

_Static_assert(__builtin_stdc_trailing_ones(0) == 0);
_Static_assert(__builtin_stdc_trailing_ones(1) == 1);
_Static_assert(__builtin_stdc_trailing_ones(2) == 0);
_Static_assert(__builtin_stdc_trailing_ones(3) == 2);
_Static_assert(__builtin_stdc_trailing_ones(1023) == 10);
_Static_assert(__builtin_stdc_trailing_ones(1024) == 0);
_Static_assert(__builtin_stdc_trailing_ones(~0u) == (8 * sizeof(int)));
_Static_assert(__builtin_stdc_trailing_ones(~0u >> 1) == (8 * sizeof(int) - 1));
_Static_assert(__builtin_stdc_trailing_ones(~0u << 1) == 0);
_Static_assert(__builtin_stdc_trailing_ones(~0ull) == (8 * sizeof(long)));
_Static_assert(__builtin_stdc_trailing_ones(~0ull >> 1) == (8 * sizeof(long) - 1));
_Static_assert(__builtin_stdc_trailing_ones(~0ull << 1) == 0);

_Static_assert(__builtin_stdc_trailing_zeros(0) == (8 * sizeof(int)));
_Static_assert(__builtin_stdc_trailing_zeros(1) == 0);
_Static_assert(__builtin_stdc_trailing_zeros(2) == 1);
_Static_assert(__builtin_stdc_trailing_zeros(3) == 0);
_Static_assert(__builtin_stdc_trailing_zeros(1023) == 0);
_Static_assert(__builtin_stdc_trailing_zeros(1024) == 10);
_Static_assert(__builtin_stdc_trailing_zeros(1u << 31) == 31);
_Static_assert(__builtin_stdc_trailing_zeros(1ull << 63) == 63);
_Static_assert(__builtin_stdc_trailing_zeros(~0u) == 0);
_Static_assert(__builtin_stdc_trailing_zeros(~1u) == 1);
_Static_assert(__builtin_stdc_trailing_zeros(~0ul) == 0);
_Static_assert(__builtin_stdc_trailing_zeros(~1ul) == 1);

int bit_ceil(long x) {
    return __builtin_stdc_bit_ceil(x);
}

int bit_floor(long x) {
    return __builtin_stdc_bit_floor(x);
}

unsigned long rotate_left(unsigned long x, unsigned char y) {
    return __builtin_stdc_rotate_left(x, y);
}

unsigned long rotate_right(unsigned long x, unsigned char y) {
    return __builtin_stdc_rotate_right(x, y);
}

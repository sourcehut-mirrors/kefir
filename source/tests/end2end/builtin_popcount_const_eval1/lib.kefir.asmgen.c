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

#define VARID(_x) x##_x
#define VARID2(_x) VARID(_x)

#define POPCOUNT_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_popcount((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_popcount((_x))

POPCOUNT_ASSERT(0, 0);
POPCOUNT_ASSERT(1, 1);
POPCOUNT_ASSERT(2, 1);
POPCOUNT_ASSERT(3, 2);
POPCOUNT_ASSERT(1 << 31, 1);
POPCOUNT_ASSERT(1ull << (8 * sizeof(int)), 0);
POPCOUNT_ASSERT(~0ull, (8 * sizeof(int)));
POPCOUNT_ASSERT(~0ull - 1, (8 * sizeof(int)) - 1);
POPCOUNT_ASSERT(~0ull - 2, (8 * sizeof(int)) - 1);
POPCOUNT_ASSERT(~0ull - 3, (8 * sizeof(int)) - 2);

#define POPCOUNTL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_popcountl((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_popcountl((_x))

POPCOUNTL_ASSERT(0, 0);
POPCOUNTL_ASSERT(1, 1);
POPCOUNTL_ASSERT(2, 1);
POPCOUNTL_ASSERT(3, 2);
POPCOUNTL_ASSERT(1ull << 31, 1);
POPCOUNTL_ASSERT(1ull << (8 * sizeof(long)), 0);
POPCOUNTL_ASSERT(~0ull, (8 * sizeof(long)));
POPCOUNTL_ASSERT(~0ull - 1, (8 * sizeof(long)) - 1);
POPCOUNTL_ASSERT(~0ull - 2, (8 * sizeof(long)) - 1);
POPCOUNTL_ASSERT(~0ull - 3, (8 * sizeof(long)) - 2);

#define POPCOUNTLL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_popcountll((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_popcountll((_x))

POPCOUNTLL_ASSERT(0, 0);
POPCOUNTLL_ASSERT(1, 1);
POPCOUNTLL_ASSERT(2, 1);
POPCOUNTLL_ASSERT(3, 2);
POPCOUNTLL_ASSERT(1ull << 31, 1);
POPCOUNTLL_ASSERT(1ull << (8 * sizeof(long long)), 0);
POPCOUNTLL_ASSERT(~0ull, (8 * sizeof(long long)));
POPCOUNTLL_ASSERT(~0ull - 1, (8 * sizeof(long long)) - 1);
POPCOUNTLL_ASSERT(~0ull - 2, (8 * sizeof(long long)) - 1);
POPCOUNTLL_ASSERT(~0ull - 3, (8 * sizeof(long long)) - 2);

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

#define PARITY_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_parity((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_parity((_x))

PARITY_ASSERT(0, 0);
PARITY_ASSERT(1, 1);
PARITY_ASSERT(2, 1);
PARITY_ASSERT(3, 0);
PARITY_ASSERT(~0ull, 0);
PARITY_ASSERT(~0ull - 1, 1);
PARITY_ASSERT(~0ull - 2, 1);
PARITY_ASSERT(~0ull - 3, 0);
PARITY_ASSERT(1ull << (8 * sizeof(int) - 1), 1);
PARITY_ASSERT((1ull << (8 * sizeof(int) - 1)) - 1, 1);
PARITY_ASSERT((1ull << (8 * sizeof(int) - 1)) - 2, 0);
PARITY_ASSERT(1ull << (8 * sizeof(int)), 0);

#define PARITYL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_parityl((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_parityl((_x))

PARITYL_ASSERT(0, 0);
PARITYL_ASSERT(1, 1);
PARITYL_ASSERT(2, 1);
PARITYL_ASSERT(3, 0);
PARITYL_ASSERT(~0ull, 0);
PARITYL_ASSERT(~0ull - 1, 1);
PARITYL_ASSERT(~0ull - 2, 1);
PARITYL_ASSERT(~0ull - 3, 0);
PARITYL_ASSERT(1ull << (8 * sizeof(long) - 1), 1);
PARITYL_ASSERT((1ull << (8 * sizeof(long) - 1)) - 1, 1);
PARITYL_ASSERT((1ull << (8 * sizeof(long) - 1)) - 2, 0);
PARITYL_ASSERT(1ull << (8 * sizeof(int)), 1);

#define PARITYLL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_parityll((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_parityll((_x))

PARITYLL_ASSERT(0, 0);
PARITYLL_ASSERT(1, 1);
PARITYLL_ASSERT(2, 1);
PARITYLL_ASSERT(3, 0);
PARITYLL_ASSERT(~0ull, 0);
PARITYLL_ASSERT(~0ull - 1, 1);
PARITYLL_ASSERT(~0ull - 2, 1);
PARITYLL_ASSERT(~0ull - 3, 0);
PARITYLL_ASSERT(1ull << (8 * sizeof(long long) - 1), 1);
PARITYLL_ASSERT((1ull << (8 * sizeof(long long) - 1)) - 1, 1);
PARITYLL_ASSERT((1ull << (8 * sizeof(long long) - 1)) - 2, 0);
PARITYLL_ASSERT(1ull << (8 * sizeof(int)), 1);

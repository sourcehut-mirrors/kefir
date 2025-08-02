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

#define CTZ_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_ctz((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_ctz((_x))
#define CTZ_ASSERT2(_x) CTZ_ASSERT(1ull << ((_x) - 1), (_x) - 1)

#define CTZ_ASSERT2_1(_x) \
    CTZ_ASSERT2((_x));    \
    CTZ_ASSERT2((_x) + 1)
#define CTZ_ASSERT2_2(_x) \
    CTZ_ASSERT2_1((_x));  \
    CTZ_ASSERT2_1((_x) + 2)
#define CTZ_ASSERT2_3(_x) \
    CTZ_ASSERT2_2((_x));  \
    CTZ_ASSERT2_2((_x) + 4)
#define CTZ_ASSERT2_4(_x) \
    CTZ_ASSERT2_3((_x));  \
    CTZ_ASSERT2_3((_x) + 8)
#define CTZ_ASSERT2_5(_x) \
    CTZ_ASSERT2_4((_x));  \
    CTZ_ASSERT2_4((_x) + 16)
CTZ_ASSERT2_5(1);
CTZ_ASSERT(~0ull, 0);

#define CTZL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_ctzl((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_ctzl((_x))
#define CTZL_ASSERT2(_x) CTZL_ASSERT(1ull << ((_x) - 1), (_x) - 1)

#define CTZL_ASSERT2_1(_x) \
    CTZL_ASSERT2((_x));    \
    CTZL_ASSERT2((_x) + 1)
#define CTZL_ASSERT2_2(_x) \
    CTZL_ASSERT2_1((_x));  \
    CTZL_ASSERT2_1((_x) + 2)
#define CTZL_ASSERT2_3(_x) \
    CTZL_ASSERT2_2((_x));  \
    CTZL_ASSERT2_2((_x) + 4)
#define CTZL_ASSERT2_4(_x) \
    CTZL_ASSERT2_3((_x));  \
    CTZL_ASSERT2_3((_x) + 8)
#define CTZL_ASSERT2_5(_x) \
    CTZL_ASSERT2_4((_x));  \
    CTZL_ASSERT2_4((_x) + 16)
#define CTZL_ASSERT2_6(_x) \
    CTZL_ASSERT2_5((_x));  \
    CTZL_ASSERT2_5((_x) + 32)
CTZL_ASSERT2_5(1);
CTZL_ASSERT(~0ull, 0);

#define CTZLL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_ctzll((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_ctzll((_x))
#define CTZLL_ASSERT2(_x) CTZLL_ASSERT(1ull << ((_x) - 1), (_x) - 1)

#define CTZLL_ASSERT2_1(_x) \
    CTZLL_ASSERT2((_x));    \
    CTZLL_ASSERT2((_x) + 1)
#define CTZLL_ASSERT2_2(_x) \
    CTZLL_ASSERT2_1((_x));  \
    CTZLL_ASSERT2_1((_x) + 2)
#define CTZLL_ASSERT2_3(_x) \
    CTZLL_ASSERT2_2((_x));  \
    CTZLL_ASSERT2_2((_x) + 4)
#define CTZLL_ASSERT2_4(_x) \
    CTZLL_ASSERT2_3((_x));  \
    CTZLL_ASSERT2_3((_x) + 8)
#define CTZLL_ASSERT2_5(_x) \
    CTZLL_ASSERT2_4((_x));  \
    CTZLL_ASSERT2_4((_x) + 16)
#define CTZLL_ASSERT2_6(_x) \
    CTZLL_ASSERT2_5((_x));  \
    CTZLL_ASSERT2_5((_x) + 32)
CTZLL_ASSERT2_5(1);
CTZLL_ASSERT(~0ull, 0);

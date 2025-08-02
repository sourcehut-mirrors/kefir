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

#define FFS_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_ffs((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_ffs((_x))
#define FFS_ASSERT2(_x) FFS_ASSERT(1ull << ((_x) - 1), (_x))

#define FFS_ASSERT2_1(_x) \
    FFS_ASSERT2((_x));    \
    FFS_ASSERT2((_x) + 1)
#define FFS_ASSERT2_2(_x) \
    FFS_ASSERT2_1((_x));  \
    FFS_ASSERT2_1((_x) + 2)
#define FFS_ASSERT2_3(_x) \
    FFS_ASSERT2_2((_x));  \
    FFS_ASSERT2_2((_x) + 4)
#define FFS_ASSERT2_4(_x) \
    FFS_ASSERT2_3((_x));  \
    FFS_ASSERT2_3((_x) + 8)
#define FFS_ASSERT2_5(_x) \
    FFS_ASSERT2_4((_x));  \
    FFS_ASSERT2_4((_x) + 16)
FFS_ASSERT(0, 0);
FFS_ASSERT2_5(1);
FFS_ASSERT(1ull << 33, 0);
FFS_ASSERT(~0ull, 1);

#define FFSL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_ffsl((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_ffsl((_x))
#define FFSL_ASSERT2(_x) FFSL_ASSERT(1ull << ((_x) - 1), (_x))

#define FFSL_ASSERT2_1(_x) \
    FFSL_ASSERT2((_x));    \
    FFSL_ASSERT2((_x) + 1)
#define FFSL_ASSERT2_2(_x) \
    FFSL_ASSERT2_1((_x));  \
    FFSL_ASSERT2_1((_x) + 2)
#define FFSL_ASSERT2_3(_x) \
    FFSL_ASSERT2_2((_x));  \
    FFSL_ASSERT2_2((_x) + 4)
#define FFSL_ASSERT2_4(_x) \
    FFSL_ASSERT2_3((_x));  \
    FFSL_ASSERT2_3((_x) + 8)
#define FFSL_ASSERT2_5(_x) \
    FFSL_ASSERT2_4((_x));  \
    FFSL_ASSERT2_4((_x) + 16)
#define FFSL_ASSERT2_6(_x) \
    FFSL_ASSERT2_5((_x));  \
    FFSL_ASSERT2_5((_x) + 32)
FFSL_ASSERT(0, 0);
FFSL_ASSERT2_6(1);
FFSL_ASSERT(~0ull, 1);

#define FFSLL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_ffsll((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_ffsll((_x))
#define FFSLL_ASSERT2(_x) FFSLL_ASSERT(1ull << ((_x) - 1), (_x))

#define FFSLL_ASSERT2_1(_x) \
    FFSLL_ASSERT2((_x));    \
    FFSLL_ASSERT2((_x) + 1)
#define FFSLL_ASSERT2_2(_x) \
    FFSLL_ASSERT2_1((_x));  \
    FFSLL_ASSERT2_1((_x) + 2)
#define FFSLL_ASSERT2_3(_x) \
    FFSLL_ASSERT2_2((_x));  \
    FFSLL_ASSERT2_2((_x) + 4)
#define FFSLL_ASSERT2_4(_x) \
    FFSLL_ASSERT2_3((_x));  \
    FFSLL_ASSERT2_3((_x) + 8)
#define FFSLL_ASSERT2_5(_x) \
    FFSLL_ASSERT2_4((_x));  \
    FFSLL_ASSERT2_4((_x) + 16)
#define FFSLL_ASSERT2_6(_x) \
    FFSLL_ASSERT2_5((_x));  \
    FFSLL_ASSERT2_5((_x) + 32)
FFSLL_ASSERT(0, 0);
FFSLL_ASSERT2_6(1);
FFSLL_ASSERT(~0ull, 1);

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

#define CLZ_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_clz((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_clz((_x))
#define CLZ_ASSERT2(_x) CLZ_ASSERT(1ull << ((_x) - 1), 8 * sizeof(int) - (_x))

#define CLZ_ASSERT2_1(_x) \
    CLZ_ASSERT2((_x));    \
    CLZ_ASSERT2((_x) + 1)
#define CLZ_ASSERT2_2(_x) \
    CLZ_ASSERT2_1((_x));  \
    CLZ_ASSERT2_1((_x) + 2)
#define CLZ_ASSERT2_3(_x) \
    CLZ_ASSERT2_2((_x));  \
    CLZ_ASSERT2_2((_x) + 4)
#define CLZ_ASSERT2_4(_x) \
    CLZ_ASSERT2_3((_x));  \
    CLZ_ASSERT2_3((_x) + 8)
#define CLZ_ASSERT2_5(_x) \
    CLZ_ASSERT2_4((_x));  \
    CLZ_ASSERT2_4((_x) + 16)
CLZ_ASSERT2_5(1);
CLZ_ASSERT(~0ull, 0);

#define CLZL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_clzl((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_clzl((_x))
#define CLZL_ASSERT2(_x) CLZL_ASSERT(1ull << ((_x) - 1), 8 * sizeof(long) - (_x))

#define CLZL_ASSERT2_1(_x) \
    CLZL_ASSERT2((_x));    \
    CLZL_ASSERT2((_x) + 1)
#define CLZL_ASSERT2_2(_x) \
    CLZL_ASSERT2_1((_x));  \
    CLZL_ASSERT2_1((_x) + 2)
#define CLZL_ASSERT2_3(_x) \
    CLZL_ASSERT2_2((_x));  \
    CLZL_ASSERT2_2((_x) + 4)
#define CLZL_ASSERT2_4(_x) \
    CLZL_ASSERT2_3((_x));  \
    CLZL_ASSERT2_3((_x) + 8)
#define CLZL_ASSERT2_5(_x) \
    CLZL_ASSERT2_4((_x));  \
    CLZL_ASSERT2_4((_x) + 16)
#define CLZL_ASSERT2_6(_x) \
    CLZL_ASSERT2_5((_x));  \
    CLZL_ASSERT2_5((_x) + 32)
CLZL_ASSERT2_6(1);
CLZL_ASSERT(~0ull, 0);

#define CLZLL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_clzll((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_clzll((_x))
#define CLZLL_ASSERT2(_x) CLZLL_ASSERT(1ull << ((_x) - 1), 8 * sizeof(long long) - (_x))

#define CLZLL_ASSERT2_1(_x) \
    CLZLL_ASSERT2((_x));    \
    CLZLL_ASSERT2((_x) + 1)
#define CLZLL_ASSERT2_2(_x) \
    CLZLL_ASSERT2_1((_x));  \
    CLZLL_ASSERT2_1((_x) + 2)
#define CLZLL_ASSERT2_3(_x) \
    CLZLL_ASSERT2_2((_x));  \
    CLZLL_ASSERT2_2((_x) + 4)
#define CLZLL_ASSERT2_4(_x) \
    CLZLL_ASSERT2_3((_x));  \
    CLZLL_ASSERT2_3((_x) + 8)
#define CLZLL_ASSERT2_5(_x) \
    CLZLL_ASSERT2_4((_x));  \
    CLZLL_ASSERT2_4((_x) + 16)
#define CLZLL_ASSERT2_6(_x) \
    CLZLL_ASSERT2_5((_x));  \
    CLZLL_ASSERT2_5((_x) + 32)
CLZLL_ASSERT2_6(1);
CLZLL_ASSERT(~0ull, 0);

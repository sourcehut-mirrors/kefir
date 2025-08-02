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

#define CLRSB_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_clrsb((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_clrsb((_x))
#define CLRSB_ASSERT2(_x) CLRSB_ASSERT(1ull << ((_x) - 1), 8 * sizeof(int) - (_x) - 1)
#define CLRSB_ASSERT3(_x) CLRSB_ASSERT(~(1ull << ((_x) - 1)), 8 * sizeof(int) - (_x) - 1)

#define CLRSB_ASSERT2_1(_x) \
    CLRSB_ASSERT2((_x));    \
    CLRSB_ASSERT2((_x) + 1)
#define CLRSB_ASSERT2_2(_x) \
    CLRSB_ASSERT2_1((_x));  \
    CLRSB_ASSERT2_1((_x) + 2)
#define CLRSB_ASSERT2_3(_x) \
    CLRSB_ASSERT2_2((_x));  \
    CLRSB_ASSERT2_2((_x) + 4)
#define CLRSB_ASSERT2_4(_x) \
    CLRSB_ASSERT2_3((_x));  \
    CLRSB_ASSERT2_3((_x) + 8)
#define CLRSB_ASSERT3_1(_x) \
    CLRSB_ASSERT3((_x));    \
    CLRSB_ASSERT3((_x) + 1)
#define CLRSB_ASSERT3_2(_x) \
    CLRSB_ASSERT3_1((_x));  \
    CLRSB_ASSERT3_1((_x) + 2)
#define CLRSB_ASSERT3_3(_x) \
    CLRSB_ASSERT3_2((_x));  \
    CLRSB_ASSERT3_2((_x) + 4)
#define CLRSB_ASSERT3_4(_x) \
    CLRSB_ASSERT3_3((_x));  \
    CLRSB_ASSERT3_3((_x) + 8)
CLRSB_ASSERT(0, 8 * sizeof(int) - 1);
CLRSB_ASSERT(~0ull, 8 * sizeof(int) - 1);
CLRSB_ASSERT2_4(1);
CLRSB_ASSERT3_4(1);

#define CLRSBL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_clrsbl((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_clrsbl((_x))
#define CLRSBL_ASSERT2(_x) CLRSBL_ASSERT(1ull << ((_x) - 1), 8 * sizeof(long) - (_x) - 1)
#define CLRSBL_ASSERT3(_x) CLRSBL_ASSERT(~(1ull << ((_x) - 1)), 8 * sizeof(long) - (_x) - 1)

#define CLRSBL_ASSERT2_1(_x) \
    CLRSBL_ASSERT2((_x));    \
    CLRSBL_ASSERT2((_x) + 1)
#define CLRSBL_ASSERT2_2(_x) \
    CLRSBL_ASSERT2_1((_x));  \
    CLRSBL_ASSERT2_1((_x) + 2)
#define CLRSBL_ASSERT2_3(_x) \
    CLRSBL_ASSERT2_2((_x));  \
    CLRSBL_ASSERT2_2((_x) + 4)
#define CLRSBL_ASSERT2_4(_x) \
    CLRSBL_ASSERT2_3((_x));  \
    CLRSBL_ASSERT2_3((_x) + 8)
#define CLRSBL_ASSERT3_1(_x) \
    CLRSBL_ASSERT3((_x));    \
    CLRSBL_ASSERT3((_x) + 1)
#define CLRSBL_ASSERT3_2(_x) \
    CLRSBL_ASSERT3_1((_x));  \
    CLRSBL_ASSERT3_1((_x) + 2)
#define CLRSBL_ASSERT3_3(_x) \
    CLRSBL_ASSERT3_2((_x));  \
    CLRSBL_ASSERT3_2((_x) + 4)
#define CLRSBL_ASSERT3_4(_x) \
    CLRSBL_ASSERT3_3((_x));  \
    CLRSBL_ASSERT3_3((_x) + 8)
CLRSBL_ASSERT(0, 8 * sizeof(long) - 1);
CLRSBL_ASSERT(~0ull, 8 * sizeof(long) - 1);
CLRSBL_ASSERT2_4(1);
CLRSBL_ASSERT3_4(1);

#define CLRSBLL_ASSERT(_x, _y)                       \
    _Static_assert(__builtin_clrsbll((_x)) == (_y)); \
    int VARID2(__COUNTER__) = __builtin_clrsbll((_x))
#define CLRSBLL_ASSERT2(_x) CLRSBLL_ASSERT(1ull << ((_x) - 1), 8 * sizeof(long long) - (_x) - 1)
#define CLRSBLL_ASSERT3(_x) CLRSBLL_ASSERT(~(1ull << ((_x) - 1)), 8 * sizeof(long long) - (_x) - 1)

#define CLRSBLL_ASSERT2_1(_x) \
    CLRSBLL_ASSERT2((_x));    \
    CLRSBLL_ASSERT2((_x) + 1)
#define CLRSBLL_ASSERT2_2(_x) \
    CLRSBLL_ASSERT2_1((_x));  \
    CLRSBLL_ASSERT2_1((_x) + 2)
#define CLRSBLL_ASSERT2_3(_x) \
    CLRSBLL_ASSERT2_2((_x));  \
    CLRSBLL_ASSERT2_2((_x) + 4)
#define CLRSBLL_ASSERT2_4(_x) \
    CLRSBLL_ASSERT2_3((_x));  \
    CLRSBLL_ASSERT2_3((_x) + 8)
#define CLRSBLL_ASSERT3_1(_x) \
    CLRSBLL_ASSERT3((_x));    \
    CLRSBLL_ASSERT3((_x) + 1)
#define CLRSBLL_ASSERT3_2(_x) \
    CLRSBLL_ASSERT3_1((_x));  \
    CLRSBLL_ASSERT3_1((_x) + 2)
#define CLRSBLL_ASSERT3_3(_x) \
    CLRSBLL_ASSERT3_2((_x));  \
    CLRSBLL_ASSERT3_2((_x) + 4)
#define CLRSBLL_ASSERT3_4(_x) \
    CLRSBLL_ASSERT3_3((_x));  \
    CLRSBLL_ASSERT3_3((_x) + 8)
CLRSBLL_ASSERT(0, 8 * sizeof(long) - 1);
CLRSBLL_ASSERT(~0ull, 8 * sizeof(long) - 1);
CLRSBLL_ASSERT2_4(1);
CLRSBLL_ASSERT3_4(1);

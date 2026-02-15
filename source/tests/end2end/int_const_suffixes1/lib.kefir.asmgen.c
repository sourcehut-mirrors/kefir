/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

#define ASSERT_TYPE(_expr, _type) _Static_assert(_Generic((_expr), _type: 1, default: 0))

ASSERT_TYPE(2147483648, long);
ASSERT_TYPE(2147483648u, unsigned int);
ASSERT_TYPE(9223372036854775807, long);
ASSERT_TYPE(9223372036854775807u, unsigned long);
ASSERT_TYPE(9223372036854775807L, long);
ASSERT_TYPE(9223372036854775808L, unsigned long);
ASSERT_TYPE(9223372036854775807uL, unsigned long);
ASSERT_TYPE(9223372036854775808uL, unsigned long);
ASSERT_TYPE(9223372036854775807LL, long long);
ASSERT_TYPE(9223372036854775808LL, unsigned long long);
ASSERT_TYPE(9223372036854775808uLL, unsigned long long);

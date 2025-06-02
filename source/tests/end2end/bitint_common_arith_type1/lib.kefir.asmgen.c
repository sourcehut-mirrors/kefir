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

#define ASSERT_TYPE(_type1, _type2, _res_type)                                        \
    _Static_assert(_Generic((((_type1) 1) + ((_type2) 2)), _res_type: 1, default: 0), \
                   "Assertion on expression type has failed")

ASSERT_TYPE(unsigned _BitInt(1), unsigned _BitInt(1), unsigned _BitInt(1));
ASSERT_TYPE(unsigned _BitInt(1), unsigned _BitInt(2), unsigned _BitInt(2));
ASSERT_TYPE(unsigned _BitInt(2), unsigned _BitInt(1), unsigned _BitInt(2));
ASSERT_TYPE(unsigned _BitInt(2), unsigned _BitInt(2), unsigned _BitInt(2));
ASSERT_TYPE(unsigned _BitInt(2), unsigned _BitInt(3), unsigned _BitInt(3));
ASSERT_TYPE(unsigned _BitInt(5), unsigned _BitInt(3), unsigned _BitInt(5));
ASSERT_TYPE(unsigned _BitInt(4), unsigned _BitInt(16), unsigned _BitInt(16));
ASSERT_TYPE(unsigned _BitInt(70), unsigned _BitInt(35), unsigned _BitInt(70));
ASSERT_TYPE(unsigned _BitInt(700), unsigned _BitInt(3500), unsigned _BitInt(3500));

ASSERT_TYPE(signed _BitInt(2), signed _BitInt(2), signed _BitInt(2));
ASSERT_TYPE(signed _BitInt(2), signed _BitInt(3), signed _BitInt(3));
ASSERT_TYPE(signed _BitInt(3), signed _BitInt(2), signed _BitInt(3));
ASSERT_TYPE(signed _BitInt(3), signed _BitInt(3), signed _BitInt(3));
ASSERT_TYPE(signed _BitInt(5), signed _BitInt(3), signed _BitInt(5));
ASSERT_TYPE(signed _BitInt(4), signed _BitInt(16), signed _BitInt(16));
ASSERT_TYPE(signed _BitInt(70), unsigned _BitInt(35), signed _BitInt(70));
ASSERT_TYPE(signed _BitInt(700), signed _BitInt(3500), signed _BitInt(3500));

ASSERT_TYPE(signed _BitInt(2), unsigned _BitInt(2), unsigned _BitInt(2));
ASSERT_TYPE(unsigned _BitInt(2), signed _BitInt(2), unsigned _BitInt(2));
ASSERT_TYPE(unsigned _BitInt(2), signed _BitInt(3), signed _BitInt(3));
ASSERT_TYPE(signed _BitInt(2), unsigned _BitInt(3), unsigned _BitInt(3));

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

#line __LINE__ "decimal_test1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
#define DECIMAL_TYPE _Decimal32
#define ALG_PREFIX test_dec32
#include "./common.h"

#define DECIMAL_TYPE _Decimal64
#define ALG_PREFIX test_dec64
#include "./common.h"

#define DECIMAL_TYPE _Decimal128
#define ALG_PREFIX test_dec128
#include "./common.h"
#endif

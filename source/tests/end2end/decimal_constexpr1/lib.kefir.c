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

#line __LINE__ "decimal_constexpr1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
__constexpr _Decimal32 a32 = 3.14159df;
__constexpr _Decimal32 b32 = 2.71828df;
__constexpr _Decimal32 c32 = a32 + b32;

__constexpr _Decimal64 a64 = 3.14159dd;
__constexpr _Decimal64 b64 = 2.71828dd;
__constexpr _Decimal64 c64 = a64 + b64;

__constexpr _Decimal128 a128 = 3.14159dl;
__constexpr _Decimal128 b128 = 2.71828dl;
__constexpr _Decimal128 c128 = a128 + b128;

_Decimal32 get32(void) {
    return c32;
}

_Decimal64 get64(void) {
    return c64;
}

_Decimal128 get128(void) {
    return c128;
}
#endif

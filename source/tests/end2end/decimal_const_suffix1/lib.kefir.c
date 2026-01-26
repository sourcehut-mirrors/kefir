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

#line __LINE__ "decimal_const_suffix1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__

_Decimal32 a = 3.14129d32;
_Decimal64 b = -2.71828d64;
_Decimal128 c = 0.319371938e5d128;
_Decimal64x d = -7536.4252d64x;

_Decimal32 mygeta(void) {
    return 3.14129d32;
}

_Decimal64 mygetb(void) {
    return -2.71828d64;
}

_Decimal128 mygetc(void) {
    return 0.319371938e5d128;
}

_Decimal64x mygetd(void) {
    return d;
}

#endif

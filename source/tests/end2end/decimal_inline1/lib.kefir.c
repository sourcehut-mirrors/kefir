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

#line __LINE__ "decimal_inline1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
inline _Decimal32 get32_helper(void) {}
inline _Decimal64 get64_helper(void) {}
inline _Decimal128 get128_helper(void) {}

_Decimal32 get32(void) {
    return get32_helper();
}

_Decimal64 get64(void) {
    return get64_helper();
}

_Decimal128 get128(void) {
    return get128_helper();
}
#endif

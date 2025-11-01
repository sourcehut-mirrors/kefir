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

#if (defined(__KEFIRCC_DECIMAL_SUPPORT__) && defined(__KEFIRCC_DECIMAL_SUPPORT_BID__) && __KEFIRCC_DECIMAL_SUPPORT__ == __KEFIRCC_DECIMAL_SUPPORT_BID__) || defined(KEFIR_END2END_ASMGEN)
__int128 i128_from_d32(_Decimal32 x) {
    return  x;
}

__int128 i128_from_d64(_Decimal64 x) {
    return  x;
}

__int128 i128_from_d128(_Decimal128 x) {
    return  x;
}

unsigned __int128 u128_from_d32(_Decimal32 x) {
    return  x;
}

unsigned __int128 u128_from_d64(_Decimal64 x) {
    return  x;
}

unsigned __int128 u128_from_d128(_Decimal128 x) {
    return  x;
}
#endif

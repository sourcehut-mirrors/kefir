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

#if (defined(__KEFIRCC_DECIMAL_SUPPORT__) && defined(__KEFIRCC_DECIMAL_SUPPORT_BID__) && defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__) && __KEFIRCC_DECIMAL_SUPPORT__ == __KEFIRCC_DECIMAL_SUPPORT_BID__) && !defined(KEFIR_PLATFORM_DECIMAL_DPD)) || defined(KEFIR_END2END_ASMGEN)
_Decimal32 i128_to_d32(__int128 x) {
    return  x;
}

_Decimal64 i128_to_d64(__int128 x) {
    return  x;
}

_Decimal128 i128_to_d128(__int128 x) {
    return  x;
}


_Decimal32 u128_to_d32(unsigned __int128 x) {
    return  x;
}

_Decimal64 u128_to_d64(unsigned __int128 x) {
    return  x;
}

_Decimal128 u128_to_d128(unsigned __int128 x) {
    return  x;
}
#endif

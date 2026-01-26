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

#if (defined(__KEFIRCC_DECIMAL_SUPPORT__) && defined(__KEFIRCC_DECIMAL_SUPPORT_BID__) && defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__) && __KEFIRCC_DECIMAL_SUPPORT__ == __KEFIRCC_DECIMAL_SUPPORT_BID__) && !defined(KEFIR_PLATFORM_DECIMAL_DPD)) || defined(KEFIR_END2END_ASMGEN)
_Decimal32 i128_to_d32(void) {
    return (__int128) -482748318319ll;
}

_Decimal64 i128_to_d64(void) {
    return (__int128) -482748318319ll;
}

_Decimal128 i128_to_d128(void) {
    return (__int128) -482748318319ll;
}

__int128 int128_from_d32(void) {
    return 3171.3181e2df;
}

__int128 int128_from_d64(void) {
    return -317831.3181e2dd;
}

__int128 int128_from_d128(void) {
    return 317831199.3181e2dl;
}

_Decimal32 u128_to_d32(void) {
    return (unsigned __int128) 482748318319ll;
}

_Decimal64 u128_to_d64(void) {
    return (unsigned __int128) 482748318319ll;
}

_Decimal128 u128_to_d128(void) {
    return (unsigned __int128) 482748318319ll;
}

unsigned __int128 uint128_from_d32(void) {
    return 3171.3181e2df;
}

unsigned __int128 uint128_from_d64(void) {
    return 317831.3181e2dd;
}

unsigned __int128 uint128_from_d128(void) {
    return 317831199.3181e2dl;
}
#endif

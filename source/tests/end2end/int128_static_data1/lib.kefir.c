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

__int128 ivalue[] = {
    10218319,
    -13839138,
    90013919u,
    999391939393919391391uwb,
    -999391939393919391391wb,
    3.14159f,
    10.31838e8,
    9913.3413L,
    3.14159f + 0.0fi,
    10.31838e8 + 0.0fi,
    9913.3413L + 0.0fi,
    3.14159fi,
    10.31838e8i,
    9913.3413Li,
    (__int128) &ivalue[1],
    (__int128) -13191993,
    (unsigned __int128) 0xcedf8edefac3ull
#if defined(__KEFIRCC_DECIMAL_SUPPORT__) && defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)
    ,
    4.131df,
    -19318.313dd,
    38193.31e3dl
#endif
};

unsigned __int128 uvalue[] = {
    10218319,
    -13839138,
    90013919u,
    999391939393919391391uwb,
    -999391939393919391391wb,
    3.14159f,
    10.31838e8,
    9913.3413L,
    3.14159f + 0.0fi,
    10.31838e8 + 0.0fi,
    9913.3413L + 0.0fi,
    3.14159fi,
    10.31838e8i,
    9913.3413Li,
    (unsigned __int128) &uvalue[1],
    (__int128) -13191993,
    (unsigned __int128) 0xcedf8edefac3ull
#if defined(__KEFIRCC_DECIMAL_SUPPORT__) && defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)
    ,
    4.131df,
    19318.313dd,
    38193.31e3dl
#endif
};

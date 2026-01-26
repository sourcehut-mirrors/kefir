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

long a[] = {(__int128) 1029, (__int128) -99182, (unsigned __int128) 991918u, (unsigned __int128) -18281988u};

unsigned long b[] = {(__int128) 1029, (__int128) -99182, (unsigned __int128) 991918u, (unsigned __int128) -18281988u};

float c[] = {(__int128) 1029, (__int128) -99182, (unsigned __int128) 991918u, (unsigned __int128) -18281988u};

double d[] = {(__int128) 1029, (__int128) -99182, (unsigned __int128) 991918u, (unsigned __int128) -18281988u};

long double e[] = {(__int128) 1029, (__int128) -99182, (unsigned __int128) 991918u, (unsigned __int128) -18281988u};

_Complex float f[] = {(__int128) 1029, (__int128) -99182, (unsigned __int128) 991918u, (unsigned __int128) -18281988u};

_Complex double g[] = {(__int128) 1029, (__int128) -99182, (unsigned __int128) 991918u, (unsigned __int128) -18281988u};

_Complex long double h[] = {(__int128) 1029, (__int128) -99182, (unsigned __int128) 991918u,
                            (unsigned __int128) -18281988u};

void *i[] = {(__int128) &h[2], (unsigned __int128) &h[3]};

_BitInt(256) j[] = {
    (__int128) -102981928139318399289281817wb,
    (unsigned __int128) 1029819281938181289281817wb,
};

unsigned _BitInt(256) k[] = {
    (__int128) -102981928139318399289281817wb,
    (unsigned __int128) 1029819281938181289281817wb,
};

#if defined(__KEFIRCC_DECIMAL_SUPPORT__) && defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)
_Decimal32 l[] = {(__int128) -3813918391, (unsigned __int128) 83818381993819304ull};

_Decimal64 m[] = {(__int128) -3813918391, (unsigned __int128) 83818381993819304ull};

_Decimal128 n[] = {(__int128) -3813918391, (unsigned __int128) 83818381993819304ull};
#endif

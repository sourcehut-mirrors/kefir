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

#line __LINE__ "decimal_const_eval_cast1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Bool x[] = {13744.55738df, 852858528.482824dd, 2482958292859528.74841dl, 0.0df, 0.0dd, 0.0dl};
long a[] = {13744.55738df, 852858528.482824dd, 2482958292859528.74841dl};
unsigned long b[] = {13744.55738df, 852858528.482824dd, 2482958292859528.74841dl};
float c[] = {13744.55738df, 852858528.482824dd, 2482958292859528.74841dl};
double d[] = {13744.55738df, 852858528.482824dd, 2482958292859528.74841dl};
long double e[] = {13744.55738df, 852858528.482824dd, 2482958292859528.74841dl};
_Complex float f[] = {13744.55738df, 852858528.482824dd, 2482958292859528.74841dl};
_Complex double g[] = {13744.55738df, 852858528.482824dd, 2482958292859528.74841dl};
_Complex long double h[] = {13744.55738df, 852858528.482824dd, 2482958292859528.74841dl};
#else
_Bool x[] = {1, 1, 1, 0, 0, 0};
long a[] = {13744, 852858528, 2482958292859528};
unsigned long b[] = {13744, 852858528, 2482958292859528};
int c[] = {1180090941, 1313560155, 1494033356};
int d[] = {-1374389535, 1087035463, 1346227501, 1103719115, -1911327471, 1126278265};
int e[] = {-1546188227, -691913360, 16396, 0, -295081433, -883533183, 16412, 0, -1683452935, -1927033744, 16434, 0};
int f[] = {1180090941, 0, 1313560155, 0, 1494033356, 0};
int g[] = {
    -1374389535, 1087035463, 0, 0, 1346227501, 1103719115, 0, 0, -1911327471, 1126278265, 0, 0,
};
int h[] = {
    -1546188227, -691913360, 16396, 0, 0,           0,           0,     0, -295081433, -883533183, 16412, 0,
    0,           0,          0,     0, -1683452935, -1927033744, 16434, 0, 0,          0,          0,     0,
};
#endif

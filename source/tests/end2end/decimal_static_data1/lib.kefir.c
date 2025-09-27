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

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Bool has_decimal = 1;
_Decimal32 x = 1234.3445df;
_Decimal64 y = 53842748.487275288dd;
_Decimal128 z = 174728758558753582875285.285782752857582dl;
#else
_Bool has_decimal = 0;
int x = 0;
int y[2] = {0};
int z[4] = {0};
#endif

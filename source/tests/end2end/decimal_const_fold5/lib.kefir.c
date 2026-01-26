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

#line __LINE__ "decimal_const_fold5"

#if defined(__KEFIRCC_DECIMAL_SUPPORT__) && defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)
_Decimal32 dec32_from_signed(void) {
    return -100000000000000000000000000000000wb;
}

_Decimal32 dec32_from_unsigned(void) {
    return 200000000000000000000000000000000uwb;
}

_Decimal64 dec64_from_signed(void) {
    return -800000000000000000000000000000000wb;
}

_Decimal64 dec64_from_unsigned(void) {
    return 4000000000000000000000000000000000uwb;
}

_Decimal128 dec128_from_signed(void) {
    return -800000000000000000000000000000000wb;
}

_Decimal128 dec128_from_unsigned(void) {
    return 4000000000000000000000000000000000uwb;
}
#endif

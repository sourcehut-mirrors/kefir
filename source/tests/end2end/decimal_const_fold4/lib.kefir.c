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

#line __LINE__ "decimal_const_fold4"

#if defined(__KEFIRCC_DECIMAL_SUPPORT__) && defined(__KEFIRCC_DECIMAL_BITINT_CONV_SUPPORT__)
_BitInt(120) dec32_to_signed(void) {
    return -67.053e2df;
}

unsigned _BitInt(120) dec32_to_unsigned(void) {
    return 901.342e1df;
}

_BitInt(120) dec64_to_signed(void) {
    return 1910.5829dd;
}

unsigned _BitInt(120) dec64_to_unsigned(void) {
    return 13819.3818dd;
}

_BitInt(120) dec128_to_signed(void) {
    return -993718.819e3dl;
}

unsigned _BitInt(120) dec128_to_unsigned(void) {
    return 381941.3819e-1dl;
}
#endif

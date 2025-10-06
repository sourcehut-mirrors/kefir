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

#line __LINE__ "decimal_const_fold2"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
float dec32_to_float(void) {
    return 3.14159df;
}

double dec32_to_double(void) {
    return 2.17828df;
}

#ifndef SKIP_LONG_DOUBLE
long double dec32_to_long_double(void) {
    return -8.53142df;
}
#endif

long dec32_to_long(void) {
    return -58.4e3df;
}

unsigned long dec32_to_ulong(void) {
    return 12.345e2df;
}

_Decimal64 dec32_to_dec64(void) {
    return 5832.52df;
}

_Decimal128 dec32_to_dec128(void) {
    return -10192.1df;
}

float dec64_to_float(void) {
    return 3.14159dd;
}

double dec64_to_double(void) {
    return 2.17828dd;
}

#ifndef SKIP_LONG_DOUBLE
long double dec64_to_long_double(void) {
    return -8.53142dd;
}
#endif

long dec64_to_long(void) {
    return -58.4e3dd;
}

unsigned long dec64_to_ulong(void) {
    return 12.345e2dd;
}

_Decimal32 dec64_to_dec32(void) {
    return 5832.52dd;
}

_Decimal128 dec64_to_dec128(void) {
    return -10192.1dd;
}

float dec128_to_float(void) {
    return 3.14159dl;
}

double dec128_to_double(void) {
    return 2.17828dl;
}

#ifndef SKIP_LONG_DOUBLE
long double dec128_to_long_double(void) {
    return -8.53142dl;
}
#endif

long dec128_to_long(void) {
    return -58.4e3dl;
}

unsigned long dec128_to_ulong(void) {
    return 12.345e2dl;
}

_Decimal32 dec128_to_dec32(void) {
    return 5832.52dl;
}

_Decimal64 dec128_to_dec64(void) {
    return -10192.1dl;
}
#endif

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

#line __LINE__ "decimal_const_fold1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 add32(void) {
    return 6.428df + 1891.1df;
}

_Decimal64 add64(void) {
    return 94291.42dd + 9103.14dd;
}

_Decimal128 add128(void) {
    return 842419.931dl + 39103.193dl;
}

_Decimal32 sub32(void) {
    return 6.428df - 1891.1df;
}

_Decimal64 sub64(void) {
    return 94291.42dd - 9103.14dd;
}

_Decimal128 sub128(void) {
    return 842419.931dl - 39103.193dl;
}

_Decimal32 mul32(void) {
    return 6.428df * 1891.1df;
}

_Decimal64 mul64(void) {
    return 94291.42dd * 9103.14dd;
}

_Decimal128 mul128(void) {
    return 842419.931dl * 39103.193dl;
}

_Decimal32 div32(void) {
    return 6.428df / 1891.1df;
}

_Decimal64 div64(void) {
    return 94291.42dd / 9103.14dd;
}

_Decimal128 div128(void) {
    return 842419.931dl / 39103.193dl;
}

_Decimal32 neg32(void) {
    return -6.428df;
}

_Decimal64 neg64(void) {
    return -94291.42dd;
}

_Decimal128 neg128(void) {
    return -842419.931dl;
}

int eq32_1(void) {
    return 6.428df == 1891.1df;
}

int eq32_2(void) {
    return 6.428df == 6.428df;
}

int eq64_1(void) {
    return 94291.42dd == 9103.14dd;
}

int eq64_2(void) {
    return 94291.42dd == 94291.42dd;
}

int eq128_1(void) {
    return 842419.931dl == 39103.193dl;
}

int eq128_2(void) {
    return 842419.931dl == 842419.931dl;
}

int gt32_1(void) {
    return 6.428df > 1891.1df;
}

int gt32_2(void) {
    return 1891.1df > 6.428df;
}

int gt64_1(void) {
    return 94291.42dd > 9103.14dd;
}

int gt64_2(void) {
    return 94291.42dd > 94291.42dd;
}

int gt128_1(void) {
    return 842419.931dl > 39103.193dl;
}

int gt128_2(void) {
    return 842419.931dl > 842419.931dl;
}

int lt32_1(void) {
    return 6.428df < 1891.1df;
}

int lt32_2(void) {
    return 1891.1df < 6.428df;
}

int lt64_1(void) {
    return 94291.42dd < 9103.14dd;
}

int lt64_2(void) {
    return 9103.14dd < 94291.42dd;
}

int lt128_1(void) {
    return 842419.931dl < 39103.193dl;
}

int lt128_2(void) {
    return 39103.193dl < 842419.931dl;
}

int isnan32_1(void) {
    return __builtin_isnan(6.428df);
}

int isnan32_2(void) {
    return __builtin_isnan(0.0df / 0.0df);
}

int isnan64_1(void) {
    return __builtin_isnan(6.428dd);
}

int isnan64_2(void) {
    return __builtin_isnan(0.0dd / 0.0dd);
}

int isnan128_1(void) {
    return __builtin_isnan(6.428dl);
}

int isnan128_2(void) {
    return __builtin_isnan(0.0dl / 0.0dl);
}
#endif

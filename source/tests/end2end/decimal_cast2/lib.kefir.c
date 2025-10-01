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

#line __LINE__ "decimal_cast2"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 int_to_d32(long x) {
    return x;
}

_Decimal32 uint_to_d32(unsigned long x) {
    return x;
}

_Decimal64 int_to_d64(long x) {
    return x;
}

_Decimal64 uint_to_d64(unsigned long x) {
    return x;
}

_Decimal128 int_to_d128(long x) {
    return x;
}

_Decimal128 uint_to_d128(unsigned long x) {
    return x;
}

long d32_to_int(_Decimal32 x) {
    return x;
}

unsigned long d32_to_uint(_Decimal32 x) {
    return x;
}

long d64_to_int(_Decimal64 x) {
    return x;
}

unsigned long d64_to_uint(_Decimal64 x) {
    return x;
}

long d128_to_int(_Decimal128 x) {
    return x;
}

unsigned long d128_to_uint(_Decimal128 x) {
    return x;
}

_Bool d32_to_bool(_Decimal32 x) {
    return x;
}

_Bool d64_to_bool(_Decimal64 x) {
    return x;
}

_Bool d128_to_bool(_Decimal128 x) {
    return x;
}

_Decimal32 bitint24_to_d32(_BitInt(24) x) {
    return x;
}

_Decimal32 bitint180_to_d32(_BitInt(180) x) {
    return x;
}

_Decimal64 bitint24_to_d64(_BitInt(24) x) {
    return x;
}

_Decimal64 bitint180_to_d64(_BitInt(180) x) {
    return x;
}

_Decimal128 bitint24_to_d128(_BitInt(24) x) {
    return x;
}

_Decimal128 bitint180_to_d128(_BitInt(180) x) {
    return x;
}

_Decimal32 ubitint24_to_d32(unsigned _BitInt(24) x) {
    return x;
}

_Decimal32 ubitint180_to_d32(unsigned _BitInt(180) x) {
    return x;
}

_Decimal64 ubitint24_to_d64(unsigned _BitInt(24) x) {
    return x;
}

_Decimal64 ubitint180_to_d64(unsigned _BitInt(180) x) {
    return x;
}

_Decimal128 ubitint24_to_d128(unsigned _BitInt(24) x) {
    return x;
}

_Decimal128 ubitint180_to_d128(unsigned _BitInt(180) x) {
    return x;
}

_BitInt(24) d32_to_bitint24(_Decimal32 x) {
    return x;
}

unsigned _BitInt(24) d32_to_ubitint24(_Decimal32 x) {
    return x;
}

_BitInt(24) d64_to_bitint24(_Decimal64 x) {
    return x;
}

unsigned _BitInt(24) d64_to_ubitint24(_Decimal64 x) {
    return x;
}

_BitInt(24) d128_to_bitint24(_Decimal128 x) {
    return x;
}

unsigned _BitInt(24) d128_to_ubitint24(_Decimal128 x) {
    return x;
}

_BitInt(180) d32_to_bitint180(_Decimal32 x) {
    return x;
}

unsigned _BitInt(180) d32_to_ubitint180(_Decimal32 x) {
    return x;
}

_BitInt(180) d64_to_bitint180(_Decimal64 x) {
    return x;
}

unsigned _BitInt(180) d64_to_ubitint180(_Decimal64 x) {
    return x;
}

_BitInt(180) d128_to_bitint180(_Decimal128 x) {
    return x;
}

unsigned _BitInt(180) d128_to_ubitint180(_Decimal128 x) {
    return x;
}
#endif

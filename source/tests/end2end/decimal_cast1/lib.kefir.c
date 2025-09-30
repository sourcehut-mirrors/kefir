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

#line __LINE__ "decimal_ops1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 d64_to_d32(_Decimal64 x) {
    return x;
}

_Decimal32 d128_to_d32(_Decimal128 x) {
    return x;
}

_Decimal64 d32_to_d64(_Decimal32 x) {
    return x;
}

_Decimal64 d128_to_d64(_Decimal128 x) {
    return x;
}

_Decimal128 d32_to_d128(_Decimal32 x) {
    return x;
}

_Decimal128 d64_to_d128(_Decimal64 x) {
    return x;
}

float d32_to_f32(_Decimal32 x) {
    return x;
}

double d32_to_f64(_Decimal32 x) {
    return x;
}

long double d32_to_f80(_Decimal32 x) {
    return x;
}

float d64_to_f32(_Decimal64 x) {
    return x;
}

double d64_to_f64(_Decimal64 x) {
    return x;
}

long double d64_to_f80(_Decimal64 x) {
    return x;
}

float d128_to_f32(_Decimal128 x) {
    return x;
}

double d128_to_f64(_Decimal128 x) {
    return x;
}

long double d128_to_f80(_Decimal128 x) {
    return x;
}

_Decimal32 f32_to_d32(float x) {
    return x;
}

_Decimal32 f64_to_d32(double x) {
    return x;
}

_Decimal32 f80_to_d32(long double x) {
    return x;
}

_Decimal64 f32_to_d64(float x) {
    return x;
}

_Decimal64 f64_to_d64(double x) {
    return x;
}

_Decimal64 f80_to_d64(long double x) {
    return x;
}

_Decimal128 f32_to_d128(float x) {
    return x;
}

_Decimal128 f64_to_d128(double x) {
    return x;
}

_Decimal128 f80_to_d128(long double x) {
    return x;
}
#endif

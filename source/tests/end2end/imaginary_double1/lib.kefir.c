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

int f64_size = sizeof(_Imaginary double);
int f64_alignment = _Alignof(_Imaginary double);
__constexpr _Imaginary double f64_const = 9.831if;
const _Imaginary double *f64_const_ptr = &f64_const;
#define CLASSIFY(_x) _Generic((_x), _Imaginary double : 1, double : 2, _Complex double : 3, default : 0)
int f64_compat[] = {
    CLASSIFY((_Imaginary double) 1.0if),
    CLASSIFY(((_Imaginary double) 1.0if) + (_Imaginary double) 2.0if),
    CLASSIFY(((_Imaginary double) 1.0if) - (_Imaginary double) 2.0if),
    CLASSIFY(((_Imaginary double) 1.0if) * (_Imaginary double) 2.0if),
    CLASSIFY(((_Imaginary double) 1.0if) / (_Imaginary double) 2.0if),
    CLASSIFY(((float) 1.0if) + (_Imaginary double) 2.0if),
    CLASSIFY(((float) 1.0if) - (_Imaginary double) 2.0if),
    CLASSIFY(((float) 1.0if) * (_Imaginary double) 2.0if),
    CLASSIFY(((float) 1.0if) / (_Imaginary double) 2.0if),
    CLASSIFY(((_Imaginary double) 1.0if) + (float) 2.0if),
    CLASSIFY(((_Imaginary double) 1.0if) - (float) 2.0if),
    CLASSIFY(((_Imaginary double) 1.0if) * (float) 2.0if),
    CLASSIFY(((_Imaginary double) 1.0if) / (float) 2.0if),
    CLASSIFY(((_Complex float) 1.0if) + (_Imaginary double) 2.0if),
    CLASSIFY(((_Complex float) 1.0if) - (_Imaginary double) 2.0if),
    CLASSIFY(((_Complex float) 1.0if) * (_Imaginary double) 2.0if),
    CLASSIFY(((_Complex float) 1.0if) / (_Imaginary double) 2.0if),
    CLASSIFY(((_Imaginary double) 1.0if) + (_Complex float) 2.0if),
    CLASSIFY(((_Imaginary double) 1.0if) - (_Complex float) 2.0if),
    CLASSIFY(((_Imaginary double) 1.0if) * (_Complex float) 2.0if),
    CLASSIFY(((_Imaginary double) 1.0if) / (_Complex float) 2.0if)
};

_Imaginary double f64_arr[] = {
    (_Imaginary double) 3.14159if,
    (float) 3.14159f,
    (_Complex float) (3.14159f + 2.71828if),
    (_Imaginary double) -3.14159i,
    (double) 3.14159,
    (_Complex double) (3.14159 + 2.71828i),
    (_Imaginary long double) -1.14159il,
    (long double) 3.24159L,
    (_Complex long double) (3.14659L + 2.71823iL),
    (_Float32) 0.5318f32,
    (_Complex _Float32) (0.5318f32 + 9420.0f32i),
    (_Float32x) 0.5318f32,
    (_Complex _Float32x) (0.5318f32 + 9420.0f32i),
    (_Float64) 0.5318f32,
    (_Complex _Float64) (0.5318f32 + 9420.0f32i),
    (_Float64x) 0.5318f32,
    (_Complex _Float64x) (0.5318f32 + 9420.0f32i),
    (_Float80) 0.5318f32,
    (_Complex _Float80) (0.5318f32 + 9420.0f32i),
    (long) -381931ll,
    (unsigned long) 982494ull
#ifdef __KEFIRCC_DECIMAL_SUPPORT__
    ,
    (_Decimal32) 8.4428df,
    (_Decimal64) 18813.42dd,
    (_Decimal64x) -18813.42d64x,
    (_Decimal128) -4829.41dl,
#else
    ,
    0.0if,
    0.0if,
    0.0if,
    0.0if
#endif
};

_Imaginary double f64_ops[] = {
    ((_Imaginary double) 3.0if) + ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) + ((float) 9.0f),
    ((float) 3.0f) + ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) + ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) + ((_Imaginary double) -3.0if),
    ((_Imaginary double) 3.1if) + (long) -5000,
    ((_Imaginary double) 3.2if) + (unsigned long) 5000,
    ((long) -5000) + ((_Imaginary double) 3.4if),
    ((unsigned long) 5000) + ((_Imaginary double) 3.5if),

    ((_Imaginary double) 3.0if) - ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) - ((float) 9.0f),
    ((float) 3.0f) - ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) - ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) - ((_Imaginary double) -3.0if),
    ((_Imaginary double) 3.1if) - (long) -5000,
    ((_Imaginary double) 3.2if) - (unsigned long) 5000,
    ((long) -5000) - ((_Imaginary double) 3.4if),
    ((unsigned long) 5000) - ((_Imaginary double) 3.5if),

    ((_Imaginary double) 3.0if) * ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) * ((float) 9.0f),
    ((float) 3.0f) * ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) * ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) * ((_Imaginary double) -3.0if),
    ((_Imaginary double) 3.1if) * (long) -5000,
    ((_Imaginary double) 3.2if) * (unsigned long) 5000,
    ((long) -5000) * ((_Imaginary double) 3.4if),
    ((unsigned long) 5000) * ((_Imaginary double) 3.5if),

    ((_Imaginary double) 3.0if) / ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) / ((float) 9.0f),
    ((float) 3.0f) / ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) / ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) / ((_Imaginary double) -3.0if),
    ((_Imaginary double) 3.1if) / (long) -5000,
    ((_Imaginary double) 3.2if) / (unsigned long) 5000,
    ((long) -5000) / ((_Imaginary double) 3.4if),
    ((unsigned long) 5000) / ((_Imaginary double) 3.5if),

    -((_Imaginary double) 3.0if),
+((_Imaginary double) -9.0if)
};

double f64_ops2[] = {
    ((_Imaginary double) 3.0if) + ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) + ((float) 9.0f),
    ((float) 3.0f) + ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) + ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) + ((_Imaginary double) -3.0if),
    ((_Imaginary double) 3.1if) + (long) -5000,
    ((_Imaginary double) 3.2if) + (unsigned long) 5000,
    ((long) -5000) + ((_Imaginary double) 3.4if),
    ((unsigned long) 5000) + ((_Imaginary double) 3.5if),

    ((_Imaginary double) 3.0if) - ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) - ((float) 9.0f),
    ((float) 3.0f) - ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) - ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) - ((_Imaginary double) -3.0if),
    ((_Imaginary double) 3.1if) - (long) -5000,
    ((_Imaginary double) 3.2if) - (unsigned long) 5000,
    ((long) -5000) - ((_Imaginary double) 3.4if),
    ((unsigned long) 5000) - ((_Imaginary double) 3.5if),

    ((_Imaginary double) 3.0if) * ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) * ((float) 9.0f),
    ((float) 3.0f) * ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) * ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) * ((_Imaginary double) -3.0if),
    ((_Imaginary double) 3.1if) * (long) -5000,
    ((_Imaginary double) 3.2if) * (unsigned long) 5000,
    ((long) -5000) * ((_Imaginary double) 3.4if),
    ((unsigned long) 5000) * ((_Imaginary double) 3.5if),

    ((_Imaginary double) 3.0if) / ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) / ((float) 9.0f),
    ((float) 3.0f) / ((_Imaginary double) 9.0if),
    ((_Imaginary double) 3.0if) / ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) / ((_Imaginary double) -3.0if),
    ((_Imaginary double) 3.1if) / (long) -5000,
    ((_Imaginary double) 3.2if) / (unsigned long) 5000,
    ((long) -5000) / ((_Imaginary double) 3.4if),
    ((unsigned long) 5000) / ((_Imaginary double) 3.5if),

    -((_Imaginary double) 3.0if),
    +((_Imaginary double) -9.0if)
};

float fi64_to_f32(_Imaginary double x) {
    return x;
}

double fi64_to_f64(_Imaginary double x) {
    return x;
}

long double fi64_to_f80(_Imaginary double x) {
    return x;
}

_Imaginary float fi64_to_fi32(_Imaginary double x) {
    return x;
}

_Imaginary double fi64_to_fi64(_Imaginary double x) {
    return x;
}

_Imaginary long double fi64_to_fi80(_Imaginary double x) {
    return x;
}

_Imaginary float f64_to_fi32(double x) {
    return x;
}

_Imaginary double f64_to_fi64(double x) {
    return x;
}

_Imaginary long double f64_to_fi80(double x) {
    return x;
}

_Complex float fi64_to_cf32(_Imaginary double x) {
    return x;
}

_Complex double fi64_to_cf64(_Imaginary double x) {
    return x;
}

_Complex long double fi64_to_cf80(_Imaginary double x) {
    return x;
}

long fi64_to_i64(_Imaginary double x) {
    return x;
}

unsigned long fi64_to_u64(_Imaginary double x) {
    return x;
}

_Imaginary double i64_to_fi64(long x) {
    return x;
}

_Imaginary double u64_to_fi64(unsigned long x) {
    return x;
}

_Imaginary double fi64_neg(_Imaginary double x) {
    return -x;
}

_Imaginary double fi64_add(_Imaginary double x, _Imaginary double y) {
    return x + y;
}

_Imaginary double fi64_sub(_Imaginary double x, _Imaginary double y) {
    return x - y;
}

double fi64_mul(_Imaginary double x, _Imaginary double y) {
    return x * y;
}

double fi64_div(_Imaginary double x, _Imaginary double y) {
    return x / y;
}

_Complex float fi64_f64_add(_Imaginary double x, double y) {
    return x + y;
}

_Complex float fi64_f64_sub(_Imaginary double x, double y) {
    return x - y;
}

_Imaginary double fi64_f64_mul(_Imaginary double x, double y) {
    return x * y;
}

_Imaginary double fi64_f64_div(_Imaginary double x, double y) {
    return x / y;
}

_Complex double f64_fi64_add(double x, _Imaginary double y) {
    return x + y;
}

_Complex double f64_fi64_sub(double x, _Imaginary double y) {
    return x - y;
}

_Imaginary double f64_fi64_mul(double x, _Imaginary double y) {
    return x * y;
}

_Imaginary double f64_fi64_div(double x, _Imaginary double y) {
    return x / y;
}

_Complex float cf64_fi64_add(_Complex double x, _Imaginary double y) {
    return x + y;
}

_Complex float cf64_fi64_sub(_Complex double x, _Imaginary double y) {
    return x - y;
}

_Complex float cf64_fi64_mul(_Complex double x, _Imaginary double y) {
    return x * y;
}

_Complex float cf64_fi64_div(_Complex double x, _Imaginary double y) {
    return x / y;
}

_Complex float fi64_cf64_add(_Imaginary double x, _Complex double y) {
    return x + y;
}

_Complex float fi64_cf64_sub(_Imaginary double x, _Complex double y) {
    return x - y;
}

_Complex float fi64_cf64_mul(_Imaginary double x, _Complex double y) {
    return x * y;
}

_Complex float fi64_cf64_div(_Imaginary double x, _Complex double y) {
    return x / y;
}

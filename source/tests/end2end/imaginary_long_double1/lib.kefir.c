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

int f80_size = sizeof(_Imaginary long double);
int f80_alignment = _Alignof(_Imaginary long double);
__constexpr _Imaginary long double f80_const = 9.831if;
const _Imaginary long double *f80_const_ptr = &f80_const;
#define CLASSIFY(_x) _Generic((_x), _Imaginary long double : 1, long double : 2, _Complex long double : 3, default : 0)
int f80_compat[] = {
    CLASSIFY((_Imaginary long double) 1.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) + (_Imaginary long double) 2.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) - (_Imaginary long double) 2.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) * (_Imaginary long double) 2.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) / (_Imaginary long double) 2.0if),
    CLASSIFY(((long double) 1.0if) + (_Imaginary long double) 2.0if),
    CLASSIFY(((long double) 1.0if) - (_Imaginary long double) 2.0if),
    CLASSIFY(((long double) 1.0if) * (_Imaginary long double) 2.0if),
    CLASSIFY(((long double) 1.0if) / (_Imaginary long double) 2.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) + (long double) 2.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) - (long double) 2.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) * (long double) 2.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) / (long double) 2.0if),
    CLASSIFY(((_Complex long double) 1.0if) + (_Imaginary long double) 2.0if),
    CLASSIFY(((_Complex long double) 1.0if) - (_Imaginary long double) 2.0if),
    CLASSIFY(((_Complex long double) 1.0if) * (_Imaginary long double) 2.0if),
    CLASSIFY(((_Complex long double) 1.0if) / (_Imaginary long double) 2.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) + (_Complex long double) 2.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) - (_Complex long double) 2.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) * (_Complex long double) 2.0if),
    CLASSIFY(((_Imaginary long double) 1.0if) / (_Complex long double) 2.0if)
};

extern long f80_iarr[] = {
    (_Imaginary long double) 54.241if,
    (_Bool) (_Imaginary long double) 0.0if,
    (_Bool) (_Imaginary long double) 9.4929if,
    (long) (_Imaginary long double) -9.4929if,
    (unsigned long) (_Imaginary long double) 109.49if
};

_Imaginary long double f80_arr[] = {
    (_Imaginary long double) 3.14159if,
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

_Imaginary long double f80_ops[] = {
    ((_Imaginary long double) 3.0if) + ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) + ((long double) 9.0f),
    ((long double) 3.0f) + ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) + ((_Complex long double) 6.0f + 9.0if),
    ((_Complex long double) 6.0f + 9.0if) + ((_Imaginary long double) -3.0if),
    ((_Imaginary long double) 3.1if) + (long) -5000,
    ((_Imaginary long double) 3.2if) + (unsigned long) 5000,
    ((long) -5000) + ((_Imaginary long double) 3.4if),
    ((unsigned long) 5000) + ((_Imaginary long double) 3.5if),

    ((_Imaginary long double) 3.0if) - ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) - ((long double) 9.0f),
    ((long double) 3.0f) - ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) - ((_Complex long double) 6.0f + 9.0if),
    ((_Complex long double) 6.0f + 9.0if) - ((_Imaginary long double) -3.0if),
    ((_Imaginary long double) 3.1if) - (long) -5000,
    ((_Imaginary long double) 3.2if) - (unsigned long) 5000,
    ((long) -5000) - ((_Imaginary long double) 3.4if),
    ((unsigned long) 5000) - ((_Imaginary long double) 3.5if),

    ((_Imaginary long double) 3.0if) * ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) * ((long double) 9.0f),
    ((long double) 3.0f) * ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) * ((_Complex long double) 6.0f + 9.0if),
    ((_Complex long double) 6.0f + 9.0if) * ((_Imaginary long double) -3.0if),
    ((_Imaginary long double) 3.1if) * (long) -5000,
    ((_Imaginary long double) 3.2if) * (unsigned long) 5000,
    ((long) -5000) * ((_Imaginary long double) 3.4if),
    ((unsigned long) 5000) * ((_Imaginary long double) 3.5if),

    ((_Imaginary long double) 3.0if) / ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) / ((long double) 9.0f),
    ((long double) 3.0f) / ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) / ((_Complex long double) 6.0f + 9.0if),
    ((_Complex long double) 6.0f + 9.0if) / ((_Imaginary long double) -3.0if),
    ((_Imaginary long double) 3.1if) / (long) -5000,
    ((_Imaginary long double) 3.2if) / (unsigned long) 5000,
    ((long) -5000) / ((_Imaginary long double) 3.4if),
    ((unsigned long) 5000) / ((_Imaginary long double) 3.5if),

    -((_Imaginary long double) 3.0if),
+((_Imaginary long double) -9.0if)
};

double f80_ops2[] = {
    ((_Imaginary long double) 3.0if) + ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) + ((long double) 9.0f),
    ((long double) 3.0f) + ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) + ((_Complex long double) 6.0f + 9.0if),
    ((_Complex long double) 6.0f + 9.0if) + ((_Imaginary long double) -3.0if),
    ((_Imaginary long double) 3.1if) + (long) -5000,
    ((_Imaginary long double) 3.2if) + (unsigned long) 5000,
    ((long) -5000) + ((_Imaginary long double) 3.4if),
    ((unsigned long) 5000) + ((_Imaginary long double) 3.5if),

    ((_Imaginary long double) 3.0if) - ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) - ((long double) 9.0f),
    ((long double) 3.0f) - ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) - ((_Complex long double) 6.0f + 9.0if),
    ((_Complex long double) 6.0f + 9.0if) - ((_Imaginary long double) -3.0if),
    ((_Imaginary long double) 3.1if) - (long) -5000,
    ((_Imaginary long double) 3.2if) - (unsigned long) 5000,
    ((long) -5000) - ((_Imaginary long double) 3.4if),
    ((unsigned long) 5000) - ((_Imaginary long double) 3.5if),

    ((_Imaginary long double) 3.0if) * ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) * ((long double) 9.0f),
    ((long double) 3.0f) * ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) * ((_Complex long double) 6.0f + 9.0if),
    ((_Complex long double) 6.0f + 9.0if) * ((_Imaginary long double) -3.0if),
    ((_Imaginary long double) 3.1if) * (long) -5000,
    ((_Imaginary long double) 3.2if) * (unsigned long) 5000,
    ((long) -5000) * ((_Imaginary long double) 3.4if),
    ((unsigned long) 5000) * ((_Imaginary long double) 3.5if),

    ((_Imaginary long double) 3.0if) / ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) / ((long double) 9.0f),
    ((long double) 3.0f) / ((_Imaginary long double) 9.0if),
    ((_Imaginary long double) 3.0if) / ((_Complex long double) 6.0f + 9.0if),
    ((_Complex long double) 6.0f + 9.0if) / ((_Imaginary long double) -3.0if),
    ((_Imaginary long double) 3.1if) / (long) -5000,
    ((_Imaginary long double) 3.2if) / (unsigned long) 5000,
    ((long) -5000) / ((_Imaginary long double) 3.4if),
    ((unsigned long) 5000) / ((_Imaginary long double) 3.5if),

    -((_Imaginary long double) 3.0if),
    +((_Imaginary long double) -9.0if)
};

float fi80_to_f32(_Imaginary long double x) {
    return x;
}

double fi80_to_f64(_Imaginary long double x) {
    return x;
}

long double fi80_to_f80(_Imaginary long double x) {
    return x;
}

_Imaginary float fi80_to_fi32(_Imaginary long double x) {
    return x;
}

_Imaginary double fi80_to_fi64(_Imaginary long double x) {
    return x;
}

_Imaginary long double fi80_to_fi80(_Imaginary long double x) {
    return x;
}

_Imaginary float f80_to_fi32(long double x) {
    return x;
}

_Imaginary double f80_to_fi64(long double x) {
    return x;
}

_Imaginary long double f80_to_fi80(long double  x) {
    return x;
}

_Complex float fi80_to_cf32(_Imaginary long double x) {
    return x;
}

_Complex double fi80_to_cf64(_Imaginary long double x) {
    return x;
}

_Complex long double fi80_to_cf80(_Imaginary long double x) {
    return x;
}

long fi80_to_i64(_Imaginary long double x) {
    return x;
}

unsigned long fi80_to_u64(_Imaginary long double x) {
    return x;
}

_Imaginary long double i64_to_fi80(long x) {
    return x;
}

_Imaginary long double u64_to_fi80(unsigned long x) {
    return x;
}

_Imaginary long double fi80_neg(_Imaginary long double x) {
    return -x;
}

_Imaginary long double fi80_add(_Imaginary long double x, _Imaginary long double y) {
    return x + y;
}

_Imaginary long double fi80_sub(_Imaginary long double x, _Imaginary long double y) {
    return x - y;
}

double fi80_mul(_Imaginary long double x, _Imaginary long double y) {
    return x * y;
}

double fi80_div(_Imaginary long double x, _Imaginary long double y) {
    return x / y;
}

_Complex long double fi80_f80_add(_Imaginary long double x, long double y) {
    return x + y;
}

_Complex long double fi80_f80_sub(_Imaginary long double x, long double y) {
    return x - y;
}

_Imaginary long double fi80_f80_mul(_Imaginary long double x, long double y) {
    return x * y;
}

_Imaginary long double fi80_f80_div(_Imaginary long double x, long double y) {
    return x / y;
}

_Complex long double f80_fi80_add(long double x, _Imaginary long double y) {
    return x + y;
}

_Complex long double f80_fi80_sub(long double x, _Imaginary long double y) {
    return x - y;
}

_Imaginary long double f80_fi80_mul(double x, _Imaginary long double y) {
    return x * y;
}

_Imaginary long double f80_fi80_div(double x, _Imaginary long double y) {
    return x / y;
}

_Complex long double cf80_fi80_add(_Complex double x, _Imaginary long double y) {
    return x + y;
}

_Complex long double cf80_fi80_sub(_Complex double x, _Imaginary long double y) {
    return x - y;
}

_Complex long double cf80_fi80_mul(_Complex double x, _Imaginary long double y) {
    return x * y;
}

_Complex long double cf80_fi80_div(_Complex double x, _Imaginary long double y) {
    return x / y;
}

_Complex long double fi80_cf80_add(_Imaginary long double x, _Complex double y) {
    return x + y;
}

_Complex long double fi80_cf80_sub(_Imaginary long double x, _Complex double y) {
    return x - y;
}

_Complex long double fi80_cf80_mul(_Imaginary long double x, _Complex double y) {
    return x * y;
}

_Complex long double fi80_cf80_div(_Imaginary long double x, _Complex double y) {
    return x / y;
}

_Bool fi80_to_bool(_Imaginary long double x) {
    return x;
}

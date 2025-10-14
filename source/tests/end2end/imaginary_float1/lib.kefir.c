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

int f32_size = sizeof(_Imaginary float);
int f32_alignment = _Alignof(_Imaginary float);
__constexpr _Imaginary float f32_const = 9.831if;
const _Imaginary float *f32_const_ptr = &f32_const;
#define CLASSIFY(_x) _Generic((_x), _Imaginary float : 1, float : 2, _Complex float : 3, default : 0)
int f32_compat[] = {
    CLASSIFY((_Imaginary float) 1.0if),
    CLASSIFY(((_Imaginary float) 1.0if) + (_Imaginary float) 2.0if),
    CLASSIFY(((_Imaginary float) 1.0if) - (_Imaginary float) 2.0if),
    CLASSIFY(((_Imaginary float) 1.0if) * (_Imaginary float) 2.0if),
    CLASSIFY(((_Imaginary float) 1.0if) / (_Imaginary float) 2.0if),
    CLASSIFY(((float) 1.0if) + (_Imaginary float) 2.0if),
    CLASSIFY(((float) 1.0if) - (_Imaginary float) 2.0if),
    CLASSIFY(((float) 1.0if) * (_Imaginary float) 2.0if),
    CLASSIFY(((float) 1.0if) / (_Imaginary float) 2.0if),
    CLASSIFY(((_Imaginary float) 1.0if) + (float) 2.0if),
    CLASSIFY(((_Imaginary float) 1.0if) - (float) 2.0if),
    CLASSIFY(((_Imaginary float) 1.0if) * (float) 2.0if),
    CLASSIFY(((_Imaginary float) 1.0if) / (float) 2.0if),
    CLASSIFY(((_Complex float) 1.0if) + (_Imaginary float) 2.0if),
    CLASSIFY(((_Complex float) 1.0if) - (_Imaginary float) 2.0if),
    CLASSIFY(((_Complex float) 1.0if) * (_Imaginary float) 2.0if),
    CLASSIFY(((_Complex float) 1.0if) / (_Imaginary float) 2.0if),
    CLASSIFY(((_Imaginary float) 1.0if) + (_Complex float) 2.0if),
    CLASSIFY(((_Imaginary float) 1.0if) - (_Complex float) 2.0if),
    CLASSIFY(((_Imaginary float) 1.0if) * (_Complex float) 2.0if),
    CLASSIFY(((_Imaginary float) 1.0if) / (_Complex float) 2.0if)
};

_Imaginary float f32_arr[] = {
    (_Imaginary float) 3.14159if,
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

float f32_arr2[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Complex float f32_arr3[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Float32 f32_arr4[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Complex _Float32 f32_arr5[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

double f64_arr2[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Complex double f64_arr3[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Float32x f64_arr4[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Complex _Float32x f64_arr5[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Float64 f64_arr6[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Complex _Float64 f64_arr7[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

long double f80_arr2[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Complex long double f80_arr3[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Float64x f80_arr4[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Complex _Float64x f80_arr5[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Float80 f80_arr6[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Complex _Float80 f80_arr7[] = {
    (_Imaginary float) 4721.7if,
    (_Imaginary double) -4721.7i,
    (_Imaginary long double) 4761.7i,
};

_Imaginary float f32_ops[] = {
    ((_Imaginary float) 3.0if) + ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) + ((float) 9.0f),
    ((float) 3.0f) + ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) + ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) + ((_Imaginary float) -3.0if),
    ((_Imaginary float) 3.1if) + (long) -5000,
    ((_Imaginary float) 3.2if) + (unsigned long) 5000,
    ((long) -5000) + ((_Imaginary float) 3.4if),
    ((unsigned long) 5000) + ((_Imaginary float) 3.5if),

    ((_Imaginary float) 3.0if) - ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) - ((float) 9.0f),
    ((float) 3.0f) - ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) - ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) - ((_Imaginary float) -3.0if),
    ((_Imaginary float) 3.1if) - (long) -5000,
    ((_Imaginary float) 3.2if) - (unsigned long) 5000,
    ((long) -5000) - ((_Imaginary float) 3.4if),
    ((unsigned long) 5000) - ((_Imaginary float) 3.5if),

    ((_Imaginary float) 3.0if) * ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) * ((float) 9.0f),
    ((float) 3.0f) * ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) * ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) * ((_Imaginary float) -3.0if),
    ((_Imaginary float) 3.1if) * (long) -5000,
    ((_Imaginary float) 3.2if) * (unsigned long) 5000,
    ((long) -5000) * ((_Imaginary float) 3.4if),
    ((unsigned long) 5000) * ((_Imaginary float) 3.5if),

    ((_Imaginary float) 3.0if) / ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) / ((float) 9.0f),
    ((float) 3.0f) / ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) / ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) / ((_Imaginary float) -3.0if),
    ((_Imaginary float) 3.1if) / (long) -5000,
    ((_Imaginary float) 3.2if) / (unsigned long) 5000,
    ((long) -5000) / ((_Imaginary float) 3.4if),
    ((unsigned long) 5000) / ((_Imaginary float) 3.5if),

    -((_Imaginary float) 3.0if),
+((_Imaginary float) -9.0if)
};

float f32_ops2[] = {
    ((_Imaginary float) 3.0if) + ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) + ((float) 9.0f),
    ((float) 3.0f) + ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) + ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) + ((_Imaginary float) -3.0if),
    ((_Imaginary float) 3.1if) + (long) -5000,
    ((_Imaginary float) 3.2if) + (unsigned long) 5000,
    ((long) -5000) + ((_Imaginary float) 3.4if),
    ((unsigned long) 5000) + ((_Imaginary float) 3.5if),

    ((_Imaginary float) 3.0if) - ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) - ((float) 9.0f),
    ((float) 3.0f) - ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) - ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) - ((_Imaginary float) -3.0if),
    ((_Imaginary float) 3.1if) - (long) -5000,
    ((_Imaginary float) 3.2if) - (unsigned long) 5000,
    ((long) -5000) - ((_Imaginary float) 3.4if),
    ((unsigned long) 5000) - ((_Imaginary float) 3.5if),

    ((_Imaginary float) 3.0if) * ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) * ((float) 9.0f),
    ((float) 3.0f) * ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) * ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) * ((_Imaginary float) -3.0if),
    ((_Imaginary float) 3.1if) * (long) -5000,
    ((_Imaginary float) 3.2if) * (unsigned long) 5000,
    ((long) -5000) * ((_Imaginary float) 3.4if),
    ((unsigned long) 5000) * ((_Imaginary float) 3.5if),

    ((_Imaginary float) 3.0if) / ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) / ((float) 9.0f),
    ((float) 3.0f) / ((_Imaginary float) 9.0if),
    ((_Imaginary float) 3.0if) / ((_Complex float) 6.0f + 9.0if),
    ((_Complex float) 6.0f + 9.0if) / ((_Imaginary float) -3.0if),
    ((_Imaginary float) 3.1if) / (long) -5000,
    ((_Imaginary float) 3.2if) / (unsigned long) 5000,
    ((long) -5000) / ((_Imaginary float) 3.4if),
    ((unsigned long) 5000) / ((_Imaginary float) 3.5if),

    -((_Imaginary float) 3.0if),
    +((_Imaginary float) -9.0if)
};

float fi32_to_f32(_Imaginary float x) {
    return x;
}

double fi32_to_f64(_Imaginary float x) {
    return x;
}

long double fi32_to_f80(_Imaginary float x) {
    return x;
}

_Imaginary float fi32_to_fi32(_Imaginary float x) {
    return x;
}

_Imaginary double fi32_to_fi64(_Imaginary float x) {
    return x;
}

_Imaginary long double fi32_to_fi80(_Imaginary float x) {
    return x;
}

_Imaginary float f32_to_fi32(float x) {
    return x;
}

_Imaginary double f32_to_fi64(float x) {
    return x;
}

_Imaginary long double f32_to_fi80(float x) {
    return x;
}

_Complex float fi32_to_cf32(_Imaginary float x) {
    return x;
}

_Complex double fi32_to_cf64(_Imaginary float x) {
    return x;
}

_Complex long double fi32_to_cf80(_Imaginary float x) {
    return x;
}

long fi32_to_i64(_Imaginary float x) {
    return x;
}

unsigned long fi32_to_u64(_Imaginary float x) {
    return x;
}

_Imaginary float i64_to_fi32(long x) {
    return x;
}

_Imaginary float u64_to_fi32(unsigned long x) {
    return x;
}

_Imaginary float fi32_neg(_Imaginary float x) {
    return -x;
}

_Imaginary float fi32_add(_Imaginary float x, _Imaginary float y) {
    return x + y;
}

_Imaginary float fi32_sub(_Imaginary float x, _Imaginary float y) {
    return x - y;
}

float fi32_mul(_Imaginary float x, _Imaginary float y) {
    return x * y;
}

float fi32_div(_Imaginary float x, _Imaginary float y) {
    return x / y;
}

_Complex float fi32_f32_add(_Imaginary float x, float y) {
    return x + y;
}

_Complex float fi32_f32_sub(_Imaginary float x, float y) {
    return x - y;
}

_Imaginary float fi32_f32_mul(_Imaginary float x, float y) {
    return x * y;
}

_Imaginary float fi32_f32_div(_Imaginary float x, float y) {
    return x / y;
}

_Complex float f32_fi32_add(float x, _Imaginary float y) {
    return x + y;
}

_Complex float f32_fi32_sub(float x, _Imaginary float y) {
    return x - y;
}

_Imaginary float f32_fi32_mul(float x, _Imaginary float y) {
    return x * y;
}

_Imaginary float f32_fi32_div(float x, _Imaginary float y) {
    return x / y;
}

_Complex float cf32_fi32_add(_Complex float x, _Imaginary float y) {
    return x + y;
}

_Complex float cf32_fi32_sub(_Complex float x, _Imaginary float y) {
    return x - y;
}

_Complex float cf32_fi32_mul(_Complex float x, _Imaginary float y) {
    return x * y;
}

_Complex float cf32_fi32_div(_Complex float x, _Imaginary float y) {
    return x / y;
}

_Complex float fi32_cf32_add(_Imaginary float x, _Complex float y) {
    return x + y;
}

_Complex float fi32_cf32_sub(_Imaginary float x, _Complex float y) {
    return x - y;
}

_Complex float fi32_cf32_mul(_Imaginary float x, _Complex float y) {
    return x * y;
}

_Complex float fi32_cf32_div(_Imaginary float x, _Complex float y) {
    return x / y;
}

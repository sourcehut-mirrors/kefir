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
int d64x_size = sizeof(_Decimal64x);
int d64x_alignment = _Alignof(_Decimal64x);
__constexpr _Decimal64x d64x_const = 2.71828;
const _Decimal64x *d64x_const_ptr = &d64x_const;
#define CLASSIFY(_x) _Generic((_x), float : 1, double : 2, long double : 3, _Decimal32 : 4, _Decimal64 : 5, _Decimal128 : 6, _Decimal64x : -1, default : 0)
int d64x_compat[] = {
    CLASSIFY((_Decimal64x) 0.0f),
    CLASSIFY(1 + (_Decimal64x) 0.0f),
    CLASSIFY(3.14df + (_Decimal64x) 0.0f),
    CLASSIFY(3.14dd + (_Decimal64x) 0.0f),
    CLASSIFY(3.14dl + (_Decimal64x) 0.0f),
    CLASSIFY(((_Decimal64x) 3.14f) + (_Decimal64x) 0.0f),
    CLASSIFY(3.14dd + (_Decimal64x) 0.0f),
    CLASSIFY(3.14dl + (_Decimal64x) 0.0f)
};

_Decimal64x d64x[] = {
    (_Decimal64x) 3.14159f,
    -((_Decimal64x) 1902.318f),
    ((_Decimal64x) 273.3f) + ((_Decimal64x) 1902.318f),
    ((_Decimal64x) 273.3f) - ((_Decimal64x) 1902.318f),
    ((_Decimal64x) 273.3f) * ((_Decimal64x) 1902.318f),
    ((_Decimal64x) 273.3f) / ((_Decimal64x) 1902.318f),
    ((_Decimal32) 273.3f) + ((_Decimal64x) 1902.318f),
    ((_Decimal32) 273.3f) - ((_Decimal64x) 1902.318f),
    ((_Decimal32) 273.3f) * ((_Decimal64x) 1902.318f),
    ((_Decimal32) 273.3f) / ((_Decimal64x) 1902.318f),
    ((_Decimal64) 273.3f) + ((_Decimal64x) 1902.318f),
    ((_Decimal64) 273.3f) - ((_Decimal64x) 1902.318f),
    ((_Decimal64) 273.3f) * ((_Decimal64x) 1902.318f),
    ((_Decimal64) 273.3f) / ((_Decimal64x) 1902.318f)
};

_Decimal64x get32x_1(void) {
    return 5.428f;
}

_Decimal64x get32x_2(void) {
    return d64x_const;
}

_Decimal64x neg32x(_Decimal64x x) {
    return -x;
}

_Decimal64x add32x(_Decimal64x x, _Decimal64x y) {
    return x + y;
}

_Decimal64x sub32x(_Decimal64x x, _Decimal64x y) {
    return x - y;
}

_Decimal64x mul32x(_Decimal64x x, _Decimal64x y) {
    return x * y;
}

_Decimal64x div32x(_Decimal64x x, _Decimal64x y) {
    return x / y;
}

_Decimal64x conv1(long x) {
    return x;
}

_Decimal64x conv2(unsigned long x) {
    return x;
}

_Decimal64x conv3(float x) {
    return x;
}

_Decimal64x conv4(double x) {
    return x;
}

_Decimal64x conv5(long double x) {
    return x;
}
#endif

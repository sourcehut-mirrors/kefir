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

int f32x_size = sizeof(_Float32x);
int f32x_alignment = _Alignof(_Float32x);
__constexpr _Float32x f32x_const = 2.71828;
const _Float32x *f32x_const_ptr = &f32x_const;
#define CLASSIFY(_x) _Generic((_x), float : 1, double : 2, long double : 3, _Decimal32 : 4, _Decimal64 : 5, _Decimal128 : 6, _Float32x : -1, default : 0)
int f32x_compat[] = {
    CLASSIFY((_Float32x) 0.0f),
    CLASSIFY(1 + (_Float32x) 0.0f),
    CLASSIFY(3.14f + (_Float32x) 0.0f),
    CLASSIFY(3.14 + (_Float32x) 0.0f),
    CLASSIFY(3.14l + (_Float32x) 0.0f),
    CLASSIFY(((_Float32x) 3.14f) + (_Float32x) 0.0f),
    CLASSIFY(3.14 + (_Float32x) 0.0f),
    CLASSIFY(3.14l + (_Float32x) 0.0f)
};

_Float32x f32x[] = {
    (_Float32x) 3.14159f,
    -((_Float32x) 1902.318f),
    ((_Float32x) 273.3f) + ((_Float32x) 1902.318f),
    ((_Float32x) 273.3f) - ((_Float32x) 1902.318f),
    ((_Float32x) 273.3f) * ((_Float32x) 1902.318f),
    ((_Float32x) 273.3f) / ((_Float32x) 1902.318f),
    (273.3f) + ((_Float32x) 1902.318f),
    (273.3f) - ((_Float32x) 1902.318f),
    (273.3f) * ((_Float32x) 1902.318f),
    (273.3f) / ((_Float32x) 1902.318f),
    ((double) 273.3f) + ((_Float32x) 1902.318f),
    ((double) 273.3f) - ((_Float32x) 1902.318f),
    ((double) 273.3f) * ((_Float32x) 1902.318f),
    ((double) 273.3f) / ((_Float32x) 1902.318f)
};

_Float32x get32x_1(void) {
    return 5.428f;
}

_Float32x get32x_2(void) {
    return f32x_const;
}

_Float32x neg32x(_Float32x x) {
    return -x;
}

_Float32x add32x(_Float32x x, _Float32x y) {
    return x + y;
}

_Float32x sub32x(_Float32x x, _Float32x y) {
    return x - y;
}

_Float32x mul32x(_Float32x x, _Float32x y) {
    return x * y;
}

_Float32x div32x(_Float32x x, _Float32x y) {
    return x / y;
}

_Float32x conv1(long x) {
    return x;
}

_Float32x conv2(unsigned long x) {
    return x;
}

_Float32x conv3(float x) {
    return x;
}

_Float32x conv4(double x) {
    return x;
}

_Float32x conv5(long double x) {
    return x;
}

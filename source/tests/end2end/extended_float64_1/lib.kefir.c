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

int f64x_size = sizeof(_Float64x);
int f64x_alignment = _Alignof(_Float64x);
__constexpr _Float64x f64x_const = 2.71828;
const _Float64x *f64x_const_ptr = &f64x_const;
#define CLASSIFY(_x) _Generic((_x), float : 1, double : 2, long double : 3, _Decimal32 : 4, _Decimal64 : 5, _Decimal128 : 6, _Float64x : -1, default : 0)
int f64x_compat[] = {
    CLASSIFY((_Float64x) 0.0f),
    CLASSIFY(1 + (_Float64x) 0.0f),
    CLASSIFY(3.14f + (_Float64x) 0.0f),
    CLASSIFY(3.14 + (_Float64x) 0.0f),
    CLASSIFY(3.14l + (_Float64x) 0.0f),
    CLASSIFY(((_Float64x) 3.14f) + (_Float64x) 0.0f),
    CLASSIFY(3.14 + (_Float64x) 0.0f),
    CLASSIFY(3.14l + (_Float64x) 0.0f)
};

_Float64x f64x[] = {
    (_Float64x) 3.14159f,
    -((_Float64x) 1902.318f),
    ((_Float64x) 273.3f) + ((_Float64x) 1902.318f),
    ((_Float64x) 273.3f) - ((_Float64x) 1902.318f),
    ((_Float64x) 273.3f) * ((_Float64x) 1902.318f),
    ((_Float64x) 273.3f) / ((_Float64x) 1902.318f),
    (273.3f) + ((_Float64x) 1902.318f),
    (273.3f) - ((_Float64x) 1902.318f),
    (273.3f) * ((_Float64x) 1902.318f),
    (273.3f) / ((_Float64x) 1902.318f),
    ((double) 273.3f) + ((_Float64x) 1902.318f),
    ((double) 273.3f) - ((_Float64x) 1902.318f),
    ((double) 273.3f) * ((_Float64x) 1902.318f),
    ((double) 273.3f) / ((_Float64x) 1902.318f)
};

_Float64x get64x_1(void) {
    return 5.428f;
}

_Float64x get64x_2(void) {
    return f64x_const;
}

_Float64x neg64x(_Float64x x) {
    return -x;
}

_Float64x add64x(_Float64x x, _Float64x y) {
    return x + y;
}

_Float64x sub64x(_Float64x x, _Float64x y) {
    return x - y;
}

_Float64x mul64x(_Float64x x, _Float64x y) {
    return x * y;
}

_Float64x div64x(_Float64x x, _Float64x y) {
    return x / y;
}

_Float64x conv1(long x) {
    return x;
}

_Float64x conv2(unsigned long x) {
    return x;
}

_Float64x conv3(float x) {
    return x;
}

_Float64x conv4(double x) {
    return x;
}

_Float64x conv5(long double x) {
    return x;
}

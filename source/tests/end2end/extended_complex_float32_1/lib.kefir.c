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

int f32_size = sizeof( _Complex _Float32x);
int f32_alignment = _Alignof(_Complex _Float32x);
__constexpr _Complex _Float32x f32_const = 2.71828 + 9.831i;
const _Complex _Float32x *f32_const_ptr = &f32_const;
#define CLASSIFY(_x) _Generic((_x), _Complex float : 1, _Complex double : 2, _Complex long double : 3, _Decimal32 : 4, _Decimal64 : 5, _Decimal128 : 6, _Complex _Float32x : -1, default : 0)
int f32_compat[] = {
    CLASSIFY((_Complex _Float32x) 0.0f - 1.0if),
    CLASSIFY(1 + (_Complex _Float32x) 0.0f + 1.0if),
    CLASSIFY(3.14f + (_Complex _Float32x) 0.0f + 2.2if),
    CLASSIFY(3.14 + (_Complex _Float32x) 0.0f - 10.1if),
    CLASSIFY(3.14l + (_Complex _Float32x) 0.0f - 9.0e5if),
    CLASSIFY(1.123if + ((_Complex _Float32x) 3.14f) + (_Complex _Float32x) 0.0f),
    CLASSIFY(3.14 + (_Complex _Float32x) 0.0f - 0.0if),
    CLASSIFY(3.14l + (_Complex _Float32x) 0.0f + 9.0if)
};

_Complex _Float32x f32[] = {
    (_Complex _Float32x) 3.14159f - 123.1if,
    -((_Complex _Float32x) 1902.318f) + 90.4if,
    ((_Complex _Float32x) 273.3f - 638.1if) + ((_Complex _Float32x) 1902.318f + 90.31if),
    ((_Complex _Float32x) 273.3f - 638.1if) - ((_Complex _Float32x) 1902.318f + 90.31if),
    ((_Complex _Float32x) 273.3f - 638.1if) * ((_Complex _Float32x) 1902.318f + 90.31if),
    ((_Complex _Float32x) 273.3f - 638.1if) / ((_Complex _Float32x) 1902.318f + 90.31if),
    (273.3f + 1.21if) + ((_Complex _Float32x) 1902.318f - 99.131if),
    (273.3f + 1.21if) - ((_Complex _Float32x) 1902.318f - 99.131if),
    (273.3f + 1.21if) * ((_Complex _Float32x) 1902.318f - 99.131if),
    (273.3f + 1.21if) / ((_Complex _Float32x) 1902.318f - 99.131if),
    ((double) 273.3f - 99.3145if) + ((_Complex _Float32x) 1902.318f + 1.0004e6if),
    ((double) 273.3f - 99.3145if) - ((_Complex _Float32x) 1902.318f + 1.0004e6if),
    ((double) 273.3f - 99.3145if) * ((_Complex _Float32x) 1902.318f + 1.0004e6if),
    ((double) 273.3f - 99.3145if) / ((_Complex _Float32x) 1902.318f + 1.0004e6if)
};

_Complex _Float32x get32_1(void) {
    return 5.428f - 8842.3if;
}

_Complex _Float32x get32_2(void) {
    return f32_const;
}

_Complex _Float32x neg32(_Complex _Float32x x) {
    return -x;
}

_Complex _Float32x add32(_Complex _Float32x x, _Complex _Float32x y) {
    return x + y;
}

_Complex _Float32x sub32(_Complex _Float32x x, _Complex _Float32x y) {
    return x - y;
}

_Complex _Float32x mul32(_Complex _Float32x x, _Complex _Float32x y) {
    return x * y;
}

_Complex _Float32x div32(_Complex _Float32x x, _Complex _Float32x y) {
    return x / y;
}

_Complex _Float32x conv1(long x) {
    return x;
}

_Complex _Float32x conv2(unsigned long x) {
    return x;
}

_Complex _Float32x conv3(float x) {
    return x;
}

_Complex _Float32x conv4(double x) {
    return x;
}

_Complex _Float32x conv5(long double x) {
    return x;
}

_Complex _Float32x conv6(_Complex float x) {
    return x;
}

_Complex _Float32x conv7(_Complex double x) {
    return x;
}

_Complex _Float32x conv8(_Complex long double x) {
    return x;
}

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

int f64_size = sizeof( _Complex _Float64);
int f64_alignment = _Alignof(_Complex _Float64);
__constexpr _Complex _Float64 f64_const = 2.71828 + 9.831i;
const _Complex _Float64 *f64_const_ptr = &f64_const;
#define CLASSIFY(_x) _Generic((_x), _Complex float : 1, _Complex double : 2, _Complex long double : 3, _Decimal32 : 4, _Decimal64 : 5, _Decimal128 : 6, _Complex _Float64 : -1, default : 0)
int f64_compat[] = {
    CLASSIFY((_Complex _Float64) 0.0f - 1.0if),
    CLASSIFY(1 + (_Complex _Float64) 0.0f + 1.0if),
    CLASSIFY(3.14f + (_Complex _Float64) 0.0f + 2.2if),
    CLASSIFY(3.14 + (_Complex _Float64) 0.0f - 10.1if),
    CLASSIFY(3.14l + (_Complex _Float64) 0.0f - 9.0e5if),
    CLASSIFY(1.123if + ((_Complex _Float64) 3.14f) + (_Complex _Float64) 0.0f),
    CLASSIFY(3.14 + (_Complex _Float64) 0.0f - 0.0if),
    CLASSIFY(3.14l + (_Complex _Float64) 0.0f + 9.0if)
};

_Complex _Float64 f64[] = {
    (_Complex _Float64) 3.14159f - 123.1if,
    -((_Complex _Float64) 1902.318f) + 90.4if,
    ((_Complex _Float64) 273.3f - 638.1if) + ((_Complex _Float64) 1902.318f + 90.31if),
    ((_Complex _Float64) 273.3f - 638.1if) - ((_Complex _Float64) 1902.318f + 90.31if),
    ((_Complex _Float64) 273.3f - 638.1if) * ((_Complex _Float64) 1902.318f + 90.31if),
    ((_Complex _Float64) 273.3f - 638.1if) / ((_Complex _Float64) 1902.318f + 90.31if),
    (273.3f + 1.21if) + ((_Complex _Float64) 1902.318f - 99.131if),
    (273.3f + 1.21if) - ((_Complex _Float64) 1902.318f - 99.131if),
    (273.3f + 1.21if) * ((_Complex _Float64) 1902.318f - 99.131if),
    (273.3f + 1.21if) / ((_Complex _Float64) 1902.318f - 99.131if),
    ((double) 273.3f - 99.3145if) + ((_Complex _Float64) 1902.318f + 1.0004e6if),
    ((double) 273.3f - 99.3145if) - ((_Complex _Float64) 1902.318f + 1.0004e6if),
    ((double) 273.3f - 99.3145if) * ((_Complex _Float64) 1902.318f + 1.0004e6if),
    ((double) 273.3f - 99.3145if) / ((_Complex _Float64) 1902.318f + 1.0004e6if)
};

_Complex _Float64 get64_1(void) {
    return 5.428f - 8842.3if;
}

_Complex _Float64 get64_2(void) {
    return f64_const;
}

_Complex _Float64 neg64(_Complex _Float64 x) {
    return -x;
}

_Complex _Float64 add64(_Complex _Float64 x, _Complex _Float64 y) {
    return x + y;
}

_Complex _Float64 sub64(_Complex _Float64 x, _Complex _Float64 y) {
    return x - y;
}

_Complex _Float64 mul64(_Complex _Float64 x, _Complex _Float64 y) {
    return x * y;
}

_Complex _Float64 div64(_Complex _Float64 x, _Complex _Float64 y) {
    return x / y;
}

_Complex _Float64 conv1(long x) {
    return x;
}

_Complex _Float64 conv2(unsigned long x) {
    return x;
}

_Complex _Float64 conv3(float x) {
    return x;
}

_Complex _Float64 conv4(double x) {
    return x;
}

_Complex _Float64 conv5(long double x) {
    return x;
}

_Complex _Float64 conv6(_Complex float x) {
    return x;
}

_Complex _Float64 conv7(_Complex double x) {
    return x;
}

_Complex _Float64 conv8(_Complex long double x) {
    return x;
}

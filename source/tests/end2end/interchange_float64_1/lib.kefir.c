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

int f64_size = sizeof(_Float64);
int f64_alignment = _Alignof(_Float64);
__constexpr _Float64 f64_const = 2.71828;
const _Float64 *f64_const_ptr = &f64_const;
#define CLASSIFY(_x)    \
    _Generic((_x),      \
        float: 1,       \
        double: 2,      \
        long double: 3, \
        _Decimal32: 4,  \
        _Decimal64: 5,  \
        _Decimal128: 6, \
        _Float64: -1,   \
        default: 0)
int f64_compat[] = {CLASSIFY((_Float64) 0.0f),         CLASSIFY(1 + (_Float64) 0.0f),
                    CLASSIFY(3.14f + (_Float64) 0.0f), CLASSIFY(3.14 + (_Float64) 0.0f),
                    CLASSIFY(3.14l + (_Float64) 0.0f), CLASSIFY(((_Float64) 3.14f) + (_Float64) 0.0f),
                    CLASSIFY(3.14 + (_Float64) 0.0f),  CLASSIFY(3.14l + (_Float64) 0.0f)};

_Float64 f64[] = {(_Float64) 3.14159f,
                  -((_Float64) 1902.318f),
                  ((_Float64) 273.3f) + ((_Float64) 1902.318f),
                  ((_Float64) 273.3f) - ((_Float64) 1902.318f),
                  ((_Float64) 273.3f) * ((_Float64) 1902.318f),
                  ((_Float64) 273.3f) / ((_Float64) 1902.318f),
                  (273.3f) + ((_Float64) 1902.318f),
                  (273.3f) - ((_Float64) 1902.318f),
                  (273.3f) * ((_Float64) 1902.318f),
                  (273.3f) / ((_Float64) 1902.318f),
                  ((double) 273.3f) + ((_Float64) 1902.318f),
                  ((double) 273.3f) - ((_Float64) 1902.318f),
                  ((double) 273.3f) * ((_Float64) 1902.318f),
                  ((double) 273.3f) / ((_Float64) 1902.318f)};

_Float64 get64_1(void) {
    return 5.428f;
}

_Float64 get64_2(void) {
    return f64_const;
}

_Float64 neg64(_Float64 x) {
    return -x;
}

_Float64 add64(_Float64 x, _Float64 y) {
    return x + y;
}

_Float64 sub64(_Float64 x, _Float64 y) {
    return x - y;
}

_Float64 mul64(_Float64 x, _Float64 y) {
    return x * y;
}

_Float64 div64(_Float64 x, _Float64 y) {
    return x / y;
}

_Float64 conv1(long x) {
    return x;
}

_Float64 conv2(unsigned long x) {
    return x;
}

_Float64 conv3(float x) {
    return x;
}

_Float64 conv4(double x) {
    return x;
}

_Float64 conv5(long double x) {
    return x;
}

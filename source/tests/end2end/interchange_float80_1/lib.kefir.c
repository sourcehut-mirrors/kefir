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

int f80_size = sizeof(_Float80);
int f80_alignment = _Alignof(_Float80);
__constexpr _Float80 f80_const = 2.71828;
const _Float80 *f80_const_ptr = &f80_const;
#define CLASSIFY(_x) _Generic((_x), float : 1, double : 2, long double : 3, _Decimal32 : 4, _Decimal64 : 5, _Decimal128 : 6, _Float80 : -1, default : 0)
int f80_compat[] = {
    CLASSIFY((_Float80) 0.0f),
    CLASSIFY(1 + (_Float80) 0.0f),
    CLASSIFY(3.14f + (_Float80) 0.0f),
    CLASSIFY(3.14 + (_Float80) 0.0f),
    CLASSIFY(3.14l + (_Float80) 0.0f),
    CLASSIFY(((_Float80) 3.14f) + (_Float80) 0.0f),
    CLASSIFY(3.14 + (_Float80) 0.0f),
    CLASSIFY(3.14l + (_Float80) 0.0f)
};

_Float80 f80[] = {
    (_Float80) 3.14159f,
    -((_Float80) 1902.318f),
    ((_Float80) 273.3f) + ((_Float80) 1902.318f),
    ((_Float80) 273.3f) - ((_Float80) 1902.318f),
    ((_Float80) 273.3f) * ((_Float80) 1902.318f),
    ((_Float80) 273.3f) / ((_Float80) 1902.318f),
    (273.3f) + ((_Float80) 1902.318f),
    (273.3f) - ((_Float80) 1902.318f),
    (273.3f) * ((_Float80) 1902.318f),
    (273.3f) / ((_Float80) 1902.318f),
    ((double) 273.3f) + ((_Float80) 1902.318f),
    ((double) 273.3f) - ((_Float80) 1902.318f),
    ((double) 273.3f) * ((_Float80) 1902.318f),
    ((double) 273.3f) / ((_Float80) 1902.318f)
};

_Float80 get80_1(void) {
    return 5.428f;
}

_Float80 get80_2(void) {
    return f80_const;
}

_Float80 neg80(_Float80 x) {
    return -x;
}

_Float80 add80(_Float80 x, _Float80 y) {
    return x + y;
}

_Float80 sub80(_Float80 x, _Float80 y) {
    return x - y;
}

_Float80 mul80(_Float80 x, _Float80 y) {
    return x * y;
}

_Float80 div80(_Float80 x, _Float80 y) {
    return x / y;
}

_Float80 conv1(long x) {
    return x;
}

_Float80 conv2(unsigned long x) {
    return x;
}

_Float80 conv3(float x) {
    return x;
}

_Float80 conv4(double x) {
    return x;
}

_Float80 conv5(long double x) {
    return x;
}

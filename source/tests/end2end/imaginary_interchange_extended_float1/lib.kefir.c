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

int f32_size = sizeof(_Imaginary _Float32);
int f32_alignment = _Alignof(_Imaginary _Float32);
int f32x_size = sizeof(_Imaginary _Float32x);
int f32x_alignment = _Alignof(_Imaginary _Float32x);
int f64_size = sizeof(_Imaginary _Float64);
int f64_alignment = _Alignof(_Imaginary _Float64);
int f64x_size = sizeof(_Imaginary _Float64x);
int f64x_alignment = _Alignof(_Imaginary _Float64x);
int f80_size = sizeof(_Imaginary _Float80);
int f80_alignment = _Alignof(_Imaginary _Float80);

#define CLASSIFY(_x)             \
    _Generic((_x),               \
        _Imaginary _Float32: 1,  \
        _Imaginary _Float32x: 2, \
        _Imaginary _Float64: 3,  \
        _Imaginary _Float64x: 4, \
        _Imaginary _Float80: 5,  \
        default: 0)
int compat[] = {CLASSIFY((_Imaginary _Float32) 1.0if),
                CLASSIFY((_Imaginary _Float32x) 1.0if),
                CLASSIFY((_Imaginary _Float64) 1.0if),
                CLASSIFY((_Imaginary _Float64x) 1.0if),
                CLASSIFY((_Imaginary _Float80) 1.0if),
                CLASSIFY(((_Imaginary _Float32) 1.0if) + (_Imaginary _Float32) 1.0if),
                CLASSIFY(((_Imaginary _Float32x) 1.0if) + (_Imaginary _Float32x) 1.0if),
                CLASSIFY(((_Imaginary _Float64) 1.0if) + (_Imaginary _Float64) 1.0if),
                CLASSIFY(((_Imaginary _Float64x) 1.0if) + (_Imaginary _Float64x) 1.0if),
                CLASSIFY(((_Imaginary _Float80) 1.0if) + (_Imaginary _Float80) 1.0if),
                CLASSIFY(((_Imaginary _Float32x) 1.0if) + (_Imaginary _Float32) 1.0if),
                CLASSIFY(((_Imaginary _Float64) 1.0if) + (_Imaginary _Float32x) 1.0if),
                CLASSIFY(((_Imaginary _Float64x) 1.0if) + (_Imaginary _Float64) 1.0if),
                CLASSIFY(((_Imaginary _Float80) 1.0if) + (_Imaginary _Float64x) 1.0if),
                CLASSIFY(((_Imaginary _Float64) 1.0if) + (_Imaginary _Float32) 1.0if),
                CLASSIFY(((_Imaginary _Float64x) 1.0if) + (_Imaginary _Float32x) 1.0if),
                CLASSIFY(((_Imaginary _Float80) 1.0if) + (_Imaginary _Float64) 1.0if),
                CLASSIFY(((_Imaginary _Float64x) 1.0if) + (_Imaginary _Float32) 1.0if),
                CLASSIFY(((_Imaginary _Float80) 1.0if) + (_Imaginary _Float32x) 1.0if),
                CLASSIFY(((_Imaginary _Float80) 1.0if) + (_Imaginary _Float32) 1.0if)};

_Imaginary _Float32 f32_const = 3.14159i;
_Imaginary _Float32x f32x_const = -3.14159i;
_Imaginary _Float64 f64_const = 2.71828i;
_Imaginary _Float64x f64x_const = -2.71828i;
_Imaginary _Float80 f80_const = 90.5429i;

_Complex _Float32 f32_op(int op, _Imaginary _Float32 x, _Imaginary _Float32 y) {
    switch (op) {
        case 0:
            return x + y;

        case 1:
            return x - y;

        case 2:
            return x * y;

        case 3:
            return x / y;

        default:
            return -x;
    }
}

_Complex _Float32x f32x_op(int op, _Imaginary _Float32x x, _Imaginary _Float32x y) {
    switch (op) {
        case 0:
            return x + y;

        case 1:
            return x - y;

        case 2:
            return x * y;

        case 3:
            return x / y;

        default:
            return -x;
    }
}

_Complex _Float64 f64_op(int op, _Imaginary _Float64 x, _Imaginary _Float64 y) {
    switch (op) {
        case 0:
            return x + y;

        case 1:
            return x - y;

        case 2:
            return x * y;

        case 3:
            return x / y;

        default:
            return -x;
    }
}

_Complex _Float64x f64x_op(int op, _Imaginary _Float64x x, _Imaginary _Float64x y) {
    switch (op) {
        case 0:
            return x + y;

        case 1:
            return x - y;

        case 2:
            return x * y;

        case 3:
            return x / y;

        default:
            return -x;
    }
}

_Complex _Float80 f80_op(int op, _Imaginary _Float80 x, _Imaginary _Float80 y) {
    switch (op) {
        case 0:
            return x + y;

        case 1:
            return x - y;

        case 2:
            return x * y;

        case 3:
            return x / y;

        default:
            return -x;
    }
}

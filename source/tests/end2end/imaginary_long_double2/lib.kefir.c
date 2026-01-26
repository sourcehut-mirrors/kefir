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

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 d32 = (_Imaginary long double) 3.14159i;
_Decimal64 d64 = (_Imaginary long double) 3.14159i;
_Decimal128 d128 = (_Imaginary long double) 3.14159i;

_Imaginary long double f32 = 3.14159df;
_Imaginary long double f64 = 3.14159dd;
_Imaginary long double f128 = 3.14159dl;

_Decimal32 fi80_to_d32(_Imaginary long double x) {
    return x;
}

_Decimal64 fi80_to_d64(_Imaginary long double x) {
    return x;
}

_Decimal128 fi80_to_d128(_Imaginary long double x) {
    return x;
}

_Imaginary long double d32_to_fi80(_Decimal32 x) {
    return x;
}

_Imaginary long double d64_to_fi80(_Decimal64 x) {
    return x;
}

_Imaginary long double d128_to_fi80(_Decimal128 x) {
    return x;
}
#endif

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

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 d32 = (_Imaginary double) 3.14159i;
_Decimal64 d64 = (_Imaginary double) 3.14159i;
_Decimal128 d128 = (_Imaginary double) 3.14159i;

_Imaginary double f32 = 3.14159df;
_Imaginary double f64 = 3.14159dd;
_Imaginary double f128 = 3.14159dl;

_Decimal32 fi64_to_d32(_Imaginary double x) {
    return x;
}

_Decimal64 fi64_to_d64(_Imaginary double x) {
    return x;
}

_Decimal128 fi64_to_d128(_Imaginary double x) {
    return x;
}

_Imaginary double d32_to_fi64(_Decimal32 x) {
    return x;
}

_Imaginary double d64_to_fi64(_Decimal64 x) {
    return x;
}

_Imaginary double d128_to_fi64(_Decimal128 x) {
    return x;
}
#endif

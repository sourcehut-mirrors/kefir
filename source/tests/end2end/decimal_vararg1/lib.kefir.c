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

#line __LINE__ "decimal_vararg1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 sum32(int x, ...) {
    __builtin_va_list args;
    __builtin_c23_va_start(args);
    _Decimal32 sum = 0.0df;
    while (x--)
        sum += __builtin_va_arg(args, _Decimal32);
    __builtin_va_end(args);
    return sum;
}

_Decimal64 sum64(int x, ...) {
    __builtin_va_list args;
    __builtin_c23_va_start(args);
    _Decimal64 sum = 0.0df;
    while (x--)
        sum += __builtin_va_arg(args, _Decimal64);
    __builtin_va_end(args);
    return sum;
}

_Decimal128 sum128(int x, ...) {
    __builtin_va_list args;
    __builtin_c23_va_start(args);
    _Decimal128 sum = 0.0df;
    while (x--)
        sum += __builtin_va_arg(args, _Decimal128);
    __builtin_va_end(args);
    return sum;
}

_Decimal128 sum128_2(int x, ...) {
    __builtin_va_list args;
    __builtin_c23_va_start(args);
    _Decimal128 sum = 0.0df;
    while (x--) {
        sum += __builtin_va_arg(args, _Decimal128);
        (void) __builtin_va_arg(args, double);
    }
    __builtin_va_end(args);
    return sum;
}
#endif

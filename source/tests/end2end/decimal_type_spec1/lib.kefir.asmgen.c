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

_Decimal32 a = 0;
_Decimal64 b = 0;
_Decimal128 c = 0;

const _Decimal32 volatile *d;
volatile _Decimal64 * e;
const _Decimal128 * const * const *f;

_Decimal32 add(_Decimal32 x, const _Decimal32 y) {
    return x + y;
}

const _Decimal64 add2(const _Decimal64 x, volatile _Decimal64 * y) {
    return x + *y;
}

const _Decimal128 *add3(const _Decimal128 x, volatile _Decimal128 * y) {
    return &(_Decimal128){x + *y};
}

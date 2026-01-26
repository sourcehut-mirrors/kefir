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

#line __LINE__ "decimal_call1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 fn1(_Decimal32, _Decimal32);
_Decimal64 fn3(_Decimal64, _Decimal64);
_Decimal128 fn5(_Decimal128, _Decimal128);

_Decimal32 test1(_Decimal32 *arr) {
    return fn1(arr[0], arr[1]);
}

_Decimal32 test2(_Decimal32 (*cb)(_Decimal32, _Decimal32), _Decimal32 *arr) {
    return cb(arr[0], arr[1]);
}

_Decimal64 test3(_Decimal64 *arr) {
    return fn3(arr[0], arr[1]);
}

_Decimal64 test4(_Decimal64 (*cb)(_Decimal64, _Decimal64), _Decimal64 *arr) {
    return cb(arr[0], arr[1]);
}

_Decimal128 test5(_Decimal128 *arr) {
    return fn5(arr[0], arr[1]);
}

_Decimal128 test6(_Decimal128 (*cb)(_Decimal128, _Decimal128), _Decimal128 *arr) {
    return cb(arr[0], arr[1]);
}
#endif

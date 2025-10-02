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

#line __LINE__ "decimal_atomic1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 load32(_Atomic _Decimal32 *ptr) {
    return *ptr;
}

_Decimal64 load64(_Atomic _Decimal64 *ptr) {
    return *ptr;
}

_Decimal128 load128(_Atomic _Decimal128 *ptr) {
    return *ptr;
}

void store32(_Atomic _Decimal32 *ptr, _Decimal32 value) {
    *ptr = value;
}

void store64(_Atomic _Decimal64 *ptr, _Decimal64 value) {
    *ptr = value;
}

void store128(_Atomic _Decimal128 *ptr, _Decimal128 value) {
    *ptr = value;
}

_Decimal32 add32(_Atomic _Decimal32 *ptr, _Decimal32 value) {
    return *ptr += value;
}

_Decimal64 add64(_Atomic _Decimal64 *ptr, _Decimal64 value) {
    return *ptr += value;
}

_Decimal128 add128(_Atomic _Decimal128 *ptr, _Decimal128 value) {
    return *ptr += value;
}
#endif

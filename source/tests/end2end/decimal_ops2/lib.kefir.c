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

#line __LINE__ "decimal_ops1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 add32(_Decimal32 *x, _Decimal32 y) {
    return *x += y;
}

_Decimal64 add64(_Decimal64 *x, _Decimal64 y) {
    return *x += y;
}

_Decimal128 add128(_Decimal128 *x, _Decimal128 y) {
    return *x += y;
}

_Decimal32 sub32(_Decimal32 *x, _Decimal32 y) {
    return *x -= y;
}

_Decimal64 sub64(_Decimal64 *x, _Decimal64 y) {
    return *x -= y;
}

_Decimal128 sub128(_Decimal128 *x, _Decimal128 y) {
    return *x -= y;
}

_Decimal32 mul32(_Decimal32 *x, _Decimal32 y) {
    return *x *= y;
}

_Decimal64 mul64(_Decimal64 *x, _Decimal64 y) {
    return *x *= y;
}

_Decimal128 mul128(_Decimal128 *x, _Decimal128 y) {
    return *x *= y;
}

_Decimal32 div32(_Decimal32 *x, _Decimal32 y) {
    return *x /= y;
}

_Decimal64 div64(_Decimal64 *x, _Decimal64 y) {
    return *x /= y;
}

_Decimal128 div128(_Decimal128 *x, _Decimal128 y) {
    return *x /= y;
}

_Decimal32 preinc32(_Decimal32 *x) {
    return ++(*x);
}

_Decimal64 preinc64(_Decimal64 *x) {
    return ++(*x);
}

_Decimal128 preinc128(_Decimal128 *x) {
    return ++(*x);
}

_Decimal32 predec32(_Decimal32 *x) {
    return --(*x);
}

_Decimal64 predec64(_Decimal64 *x) {
    return --(*x);
}

_Decimal128 predec128(_Decimal128 *x) {
    return --(*x);
}

_Decimal32 postinc32(_Decimal32 *x) {
    return (*x)++;
}

_Decimal64 postinc64(_Decimal64 *x) {
    return (*x)++;
}

_Decimal128 postinc128(_Decimal128 *x) {
    return (*x)++;
}

_Decimal32 postdec32(_Decimal32 *x) {
    return (*x)--;
}

_Decimal64 postdec64(_Decimal64 *x) {
    return (*x)--;
}

_Decimal128 postdec128(_Decimal128 *x) {
    return (*x)--;
}
#endif

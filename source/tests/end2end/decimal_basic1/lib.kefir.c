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

#line __LINE__ "decimal_constants1"

#ifdef __KEFIRCC_DECIMAL_SUPPORT__
_Decimal32 get32(void) {
    return 30.14159df;
}

_Decimal64 get64(void) {
    return 2274.31884dd;
}

_Decimal128 get128(void) {
    return 4818418.471847dl;
}

_Decimal32 arg32(_Decimal32 x) {
    return x;
}

_Decimal64 arg64(_Decimal64 x) {
    return x;
}

_Decimal128 arg128(_Decimal128 x) {
    return x;
}

_Decimal32 arg32x(int i, _Decimal32 x, _Decimal32 y, _Decimal32 z, _Decimal32 w, _Decimal32 a, _Decimal32 b, _Decimal32 c, _Decimal32 d, _Decimal32 e, _Decimal32 f, _Decimal32 g, _Decimal32 h) {
    switch (i) {
        case 0:
            return x;

        case 1:
            return y;

        case 2:
            return z;

        case 3:
            return w;

        case 4:
            return a;

        case 5:
            return b;

        case 6:
            return c;

        case 7:
            return d;

        case 8:
            return e;

        case 9:
            return f;

        case 10:
            return g;

        case 11:
            return h;
    }
}

_Decimal64 arg64x(int i, _Decimal64 x, _Decimal64 y, _Decimal64 z, _Decimal64 w, _Decimal64 a, _Decimal64 b, _Decimal64 c, _Decimal64 d, _Decimal64 e, _Decimal64 f, _Decimal64 g, _Decimal64 h) {
    switch (i) {
        case 0:
            return x;

        case 1:
            return y;

        case 2:
            return z;

        case 3:
            return w;

        case 4:
            return a;

        case 5:
            return b;

        case 6:
            return c;

        case 7:
            return d;

        case 8:
            return e;

        case 9:
            return f;

        case 10:
            return g;

        case 11:
            return h;
    }
}

_Decimal128 arg128x(int i, _Decimal128 x, _Decimal128 y, _Decimal128 z, _Decimal128 w, _Decimal128 a, _Decimal128 b, _Decimal128 c, _Decimal128 d, _Decimal128 e, _Decimal128 f, _Decimal128 g, _Decimal128 h) {
    switch (i) {
        case 0:
            return x;

        case 1:
            return y;

        case 2:
            return z;

        case 3:
            return w;

        case 4:
            return a;

        case 5:
            return b;

        case 6:
            return c;

        case 7:
            return d;

        case 8:
            return e;

        case 9:
            return f;

        case 10:
            return g;

        case 11:
            return h;
    }
}

_Decimal32 load32(_Decimal32 *ptr) {
    return *ptr;
}

_Decimal64 load64(_Decimal64 *ptr) {
    return *ptr;
}

_Decimal128 load128(_Decimal128 *ptr) {
    return *ptr;
}

void store32(_Decimal32 *ptr, _Decimal32 value) {
    *ptr = value;
}

void store64(_Decimal64 *ptr, _Decimal64 value) {
    *ptr = value;
}

void store128(_Decimal128 *ptr, _Decimal128 value) {
    *ptr = value;
}
#endif

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

struct struct32 {
    _Decimal32 x;
    _Decimal32 y;
    _Decimal32 z;
    _Decimal32 w;
    _Decimal32 a;
    _Decimal32 b;
    _Decimal32 c;
    _Decimal32 d;
    _Decimal32 e;
    _Decimal32 f;
    _Decimal32 g;
    _Decimal32 h;
};

struct struct64 {
    _Decimal64 x;
    _Decimal64 y;
    _Decimal64 z;
    _Decimal64 w;
    _Decimal64 a;
    _Decimal64 b;
    _Decimal64 c;
    _Decimal64 d;
    _Decimal64 e;
    _Decimal64 f;
    _Decimal64 g;
    _Decimal64 h;
};

struct struct128 {
    _Decimal128 x;
    _Decimal128 y;
    _Decimal128 z;
    _Decimal128 w;
    _Decimal128 a;
    _Decimal128 b;
    _Decimal128 c;
    _Decimal128 d;
    _Decimal128 e;
    _Decimal128 f;
    _Decimal128 g;
    _Decimal128 h;
};

_Decimal32 arg32y(int i, struct struct32 arg) {
    switch (i) {
        case 0:
            return arg.x;

        case 1:
            return arg.y;

        case 2:
            return arg.z;

        case 3:
            return arg.w;

        case 4:
            return arg.a;

        case 5:
            return arg.b;

        case 6:
            return arg.c;

        case 7:
            return arg.d;

        case 8:
            return arg.e;

        case 9:
            return arg.f;

        case 10:
            return arg.g;

        case 11:
            return arg.h;
    }
}

_Decimal64 arg64y(int i, struct struct64 arg) {
    switch (i) {
        case 0:
            return arg.x;

        case 1:
            return arg.y;

        case 2:
            return arg.z;

        case 3:
            return arg.w;

        case 4:
            return arg.a;

        case 5:
            return arg.b;

        case 6:
            return arg.c;

        case 7:
            return arg.d;

        case 8:
            return arg.e;

        case 9:
            return arg.f;

        case 10:
            return arg.g;

        case 11:
            return arg.h;
    }
}

_Decimal128 arg128y(int i, struct struct128 arg) {
    switch (i) {
        case 0:
            return arg.x;

        case 1:
            return arg.y;

        case 2:
            return arg.z;

        case 3:
            return arg.w;

        case 4:
            return arg.a;

        case 5:
            return arg.b;

        case 6:
            return arg.c;

        case 7:
            return arg.d;

        case 8:
            return arg.e;

        case 9:
            return arg.f;

        case 10:
            return arg.g;

        case 11:
            return arg.h;
    }
}

struct struct32 ret32(_Decimal32 x, _Decimal32 y, _Decimal32 z, _Decimal32 w, _Decimal32 a, _Decimal32 b, _Decimal32 c, _Decimal32 d, _Decimal32 e, _Decimal32 f, _Decimal32 g, _Decimal32 h) {
    return (struct struct32) {
        x, y, z, w, a, b, c, d, e, f, g, h
    };
}

struct struct64 ret64(_Decimal64 x, _Decimal64 y, _Decimal64 z, _Decimal64 w, _Decimal64 a, _Decimal64 b, _Decimal64 c, _Decimal64 d, _Decimal64 e, _Decimal64 f, _Decimal64 g, _Decimal64 h) {
    return (struct struct64) {
        x, y, z, w, a, b, c, d, e, f, g, h
    };
}

struct struct128 ret128(_Decimal128 x, _Decimal128 y, _Decimal128 z, _Decimal128 w, _Decimal128 a, _Decimal128 b, _Decimal128 c, _Decimal128 d, _Decimal128 e, _Decimal128 f, _Decimal128 g, _Decimal128 h) {
    return (struct struct128) {
        x, y, z, w, a, b, c, d, e, f, g, h
    };
}

struct struct32_2 {
    _Decimal32 a;
    _Decimal32 b;
};

struct struct64_2 {
    _Decimal64 a;
    _Decimal64 b;
};

struct struct128_2 {
    _Decimal128 a;
    _Decimal128 b;
};

struct struct32_2 swap32(struct struct32_2 x) {
    return (struct struct32_2) {
        x.b, x.a
    };
}

struct struct64_2 swap64(struct struct64_2 x) {
    return (struct struct64_2) {
        x.b, x.a
    };
}

struct struct128_2 swap128(struct struct128_2 x) {
    return (struct struct128_2) {
        x.b, x.a
    };
}
#endif

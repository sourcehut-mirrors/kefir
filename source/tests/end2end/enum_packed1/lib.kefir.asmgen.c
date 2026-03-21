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

#define ASSERT_TYPE(_expr, _type) _Static_assert(_Generic(_expr, _type: 1, default: 0))

enum A { A1, A2, A3 } __attribute__((packed));

ASSERT_TYPE((enum A) A1, unsigned char);
ASSERT_TYPE(A1, int);
_Static_assert(sizeof(enum A) == sizeof(char));
_Static_assert(_Alignof(enum A) == _Alignof(char));
_Static_assert(sizeof(A1) == sizeof(int));
_Static_assert(_Alignof(A1) == _Alignof(int));

enum B { B1 = -1, B2, B3 } __attribute__((packed));

ASSERT_TYPE((enum B) B1, signed char);
ASSERT_TYPE(B1, int);
_Static_assert(sizeof(enum B) == sizeof(char));
_Static_assert(_Alignof(enum B) == _Alignof(char));
_Static_assert(sizeof(B1) == sizeof(int));
_Static_assert(_Alignof(B1) == _Alignof(int));

enum C { C1 = 255, C2, C3 } __attribute__((packed));

ASSERT_TYPE((enum C) C1, unsigned short);
ASSERT_TYPE(C1, int);
_Static_assert(sizeof(enum C) == sizeof(short));
_Static_assert(_Alignof(enum C) == _Alignof(short));
_Static_assert(sizeof(C1) == sizeof(int));
_Static_assert(_Alignof(C1) == _Alignof(int));

enum D { D1 = -300, D2, D3 } __attribute__((packed));

ASSERT_TYPE((enum D) D1, signed short);
ASSERT_TYPE(D1, int);
_Static_assert(sizeof(enum D) == sizeof(short));
_Static_assert(_Alignof(enum D) == _Alignof(short));
_Static_assert(sizeof(D1) == sizeof(int));
_Static_assert(_Alignof(D1) == _Alignof(int));

enum E { E1 = 65536, E2, E3 } __attribute__((packed));

ASSERT_TYPE((enum E) E1, unsigned int);
ASSERT_TYPE(E1, int);
_Static_assert(sizeof(enum E) == sizeof(int));
_Static_assert(_Alignof(enum E) == _Alignof(int));
_Static_assert(sizeof(E1) == sizeof(int));
_Static_assert(_Alignof(E1) == _Alignof(int));

enum F { F1 = -66000, F2, F3 } __attribute__((packed));

ASSERT_TYPE((enum F) F1, int);
ASSERT_TYPE(F1, int);
_Static_assert(sizeof(enum F) == sizeof(int));
_Static_assert(_Alignof(enum F) == _Alignof(int));
_Static_assert(sizeof(F1) == sizeof(int));
_Static_assert(_Alignof(F1) == _Alignof(int));

enum G { G1 = (1u << 31) - 1, G2, G3 } __attribute__((packed));

ASSERT_TYPE((enum G) G1, unsigned int);
ASSERT_TYPE(G1, unsigned int);
_Static_assert(sizeof(enum G) == sizeof(int));
_Static_assert(_Alignof(enum G) == _Alignof(int));
_Static_assert(sizeof(G1) == sizeof(int));
_Static_assert(_Alignof(G1) == _Alignof(int));

enum H { H1 = ~0u, H2, H3 } __attribute__((packed));

ASSERT_TYPE((enum H) H1, unsigned long);
ASSERT_TYPE(H1, unsigned long);
_Static_assert(sizeof(enum H) == sizeof(long));
_Static_assert(_Alignof(enum H) == _Alignof(long));
_Static_assert(sizeof(H1) == sizeof(long));
_Static_assert(_Alignof(H1) == _Alignof(long));

enum I { I1 = -1ll * (~0u), I2, I3 } __attribute__((packed));

ASSERT_TYPE((enum I) I1, long);
ASSERT_TYPE(I1, long);
_Static_assert(sizeof(enum I) == sizeof(long));
_Static_assert(_Alignof(enum I) == _Alignof(long));
_Static_assert(sizeof(I1) == sizeof(long));
_Static_assert(_Alignof(I1) == _Alignof(long));

enum J { J1 = ~0ull, J2, J3 } __attribute__((packed));

ASSERT_TYPE((enum J) J1, unsigned long);
ASSERT_TYPE(J1, unsigned long);
_Static_assert(sizeof(enum J) == sizeof(long));
_Static_assert(_Alignof(enum J) == _Alignof(long));
_Static_assert(sizeof(J1) == sizeof(long));
_Static_assert(_Alignof(J1) == _Alignof(long));

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

#include "./definitions.h"

enum A { A1, A2, A3 };

enum B { B1 = -1, B2, B3 };

enum C { C1 = __INT_MAX__ - 1, C2 };

enum D { D1 = __INT_MAX__, D2 };

enum E { E1 = (1ull << 32) - 2, E2 };

enum F { F1 = (1ull << 32) - 2, F2, F3 };

enum G { G1 = (1ull << 32) - 2, G2, G3 = -1 };

enum H { H1 = __LONG_MAX__ - 1, H2 };

enum I { I1 = __LONG_MAX__, I2 };

enum J { J1 = __LONG_MAX__, J2 = -1 };

enum K { K1 = __LONG_MAX__, K2, K3 };

enum L { L1 = ~0ull, L2 };

#define CHECK(_x) _Generic((_x), int: 1, unsigned int: 2, long: 3, unsigned long: 4, default: 0)

int arr[] = {CHECK(A1), CHECK((enum A) A1), sizeof(A1), sizeof(enum A),

             CHECK(B1), CHECK((enum B) B1), sizeof(B1), sizeof(enum B),

             CHECK(C1), CHECK((enum C) C1), sizeof(C1), sizeof(enum C),

             CHECK(D1), CHECK((enum D) D1), sizeof(D1), sizeof(enum D),

             CHECK(D1), CHECK((enum E) E1), sizeof(E1), sizeof(enum E),

             CHECK(F1), CHECK((enum F) F1), sizeof(F1), sizeof(enum F),

             CHECK(G1), CHECK((enum G) G1), sizeof(G1), sizeof(enum G),

             CHECK(H1), CHECK((enum H) H1), sizeof(H1), sizeof(enum H),

             CHECK(I1), CHECK((enum I) I1), sizeof(I1), sizeof(enum I),

             CHECK(J1), CHECK((enum J) J1), sizeof(J1), sizeof(enum J),

             CHECK(K1), CHECK((enum K) K1), sizeof(K1), sizeof(enum K),

             CHECK(L1), CHECK((enum L) L1), sizeof(L1), sizeof(enum L)};

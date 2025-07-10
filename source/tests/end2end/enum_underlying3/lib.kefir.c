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

enum A { A1 };

enum B { B1 };

enum C { C1 = __INT_MAX__, C2 };

enum D { D1 = __INT_MAX__, D2 };

enum E { E1 = (1ull << 32) - 1, E2 };

enum F { F1 = (1ull << 32) - 1, F2 };

enum G { G1 = ~0ull };

enum H { H1 = ~0ull };

enum I : char { I1 };

enum J : char { J1 };

enum K : unsigned int { K1 };

enum L : unsigned int { L1 };

int arr[] = {_Generic(A1, __typeof__(B1): 1, default: 0), _Generic(B1, __typeof__(A1): 1, default: 0),

             _Generic(C1, __typeof__(D1): 1, default: 0), _Generic(D1, __typeof__(C1): 1, default: 0),
             _Generic(C1, __typeof__(C2): 1, default: 0), _Generic(D1, __typeof__(D2): 1, default: 0),

             _Generic(E1, __typeof__(F1): 1, default: 0), _Generic(F1, __typeof__(E1): 1, default: 0),
             _Generic(E1, __typeof__(E2): 1, default: 0), _Generic(F1, __typeof__(F2): 1, default: 0),

             _Generic(G1, __typeof__(H1): 1, default: 0), _Generic(H1, __typeof__(G1): 1, default: 0),

             _Generic(I1, __typeof__(J1): 1, default: 0), _Generic(J1, __typeof__(I1): 1, default: 0),
             _Generic(I1, __typeof__(I1): 1, default: 0), _Generic(J1, __typeof__(J1): 1, default: 0),

             _Generic(K1, __typeof__(L1): 1, default: 0), _Generic(L1, __typeof__(K1): 1, default: 0),
             _Generic(K1, __typeof__(K1): 1, default: 0), _Generic(L1, __typeof__(L1): 1, default: 0)};

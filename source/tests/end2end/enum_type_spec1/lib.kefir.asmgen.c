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

#define DEF(_id, _type)                         \
    enum _id : _type { _id##1 };                \
    enum _id##2 : const _type{_id##2};          \
    enum _id##3 : volatile _type{_id##3};       \
    enum _id##5 : volatile const _type{_id##5}; \
    enum _id##6 : _Atomic _type{_id##6}

DEF(A, char);
DEF(B, unsigned char);
DEF(C, signed char);
DEF(D, short);
DEF(E, unsigned short);
DEF(F, signed short);
DEF(G, int);
DEF(H, unsigned int);
DEF(I, signed int);
DEF(J, long);
DEF(K, unsigned long);
DEF(L, signed long);
DEF(M, long long);
DEF(N, unsigned long long);
DEF(O, signed long long);

enum X : __typeof__(100) { X1 };

enum Y : __typeof__(100ll) { Y1 };

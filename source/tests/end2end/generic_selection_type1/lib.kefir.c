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

enum A : short { A1 };

int arr[] = {_Generic(int, int: 1, long: 2, default: 0),
             _Generic(long, int: 1, long: 2, default: 0),
             _Generic(const int, int: 1, long: 2, default: 0),
             _Generic(const int, const int: 3, int: 1, long: 2, default: 0),
             _Generic(enum A, short: 1, int: 2, default: 0),
             _Generic(
                 struct B { int x : 21; }, int: 1, struct B { int x : 21; }: 2),
             _Generic(enum A, enum A: short{A1}: 1, int: 2, default: 0)};

int test1(int x) {
    switch (x) {
        case 0:
            return _Generic(int, int: 1, long: 2, default: 0);

        case 1:
            return _Generic(long, int: 1, long: 2, default: 0);

        case 2:
            return _Generic(const int, int: 1, long: 2, default: 0);

        case 3:
            return _Generic(const int, const int: 3, int: 1, long: 2, default: 0);

        case 4:
            return _Generic(enum A, short: 1, int: 2, default: 0);

        case 5:
            return _Generic(struct B { int x : 21; }, int: 1, struct B { int x : 21; }: 2);

        case 6:
            return _Generic(enum A, enum A: short{A1}: 1, int: 2, default: 0);
    }
    return -1;
}

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

ENUM;
typedef ENUM enum1_t;

ENUM get(int);

enum enum1 { CONST1 = CONST1, CONST2, CONST3 = CONST3, CONST4 = CONST3 + 100, CONST5 = -CONST4 };

typedef enum enum1 { CONST1 = CONST1, CONST2, CONST3 = CONST3, CONST4 = CONST3 + 100, CONST5 = -CONST4 } enum1_t;

enum enum1 get(int x) {
    switch (x) {
        case 0:
            return CONST1;

        case 1:
            return CONST2;

        case 2:
            return CONST3;

        case 3:
            return CONST4;

        default:
            return CONST5;
    }
}
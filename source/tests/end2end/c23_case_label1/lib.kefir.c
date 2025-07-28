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

int test1(int x) {
    switch (x) {
        case 0:
            [[fallthrough]];

        case 1:
            return ~x;

        case 2:
    }

    return -x;
}

int test2(int x) {
    switch (x) {
        case 1 ... 9:
            [[fallthrough]];

        case 10:
            return -x;

        default:
    }

    return x;
}

int test3(int x) {
    switch (x) {
        case 10:
            [[fallthrough]];

        default:
            return -x;

        case 0 ... 9:
    }

    return x;
}

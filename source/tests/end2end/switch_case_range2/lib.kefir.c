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

#include "definitions.h"

int test1(unsigned char x) {
    switch (x) {
        case 0xf0 ... 0xff:
            return 1;

        default:
            return 2;
    }
}

int test2(unsigned short x) {
    switch (x) {
        case 0xff00 ... 0xffff:
            return 1;

        default:
            return 2;
    }
}

int test3(unsigned int x) {
    switch (x) {
        case 0xffffff00u ... 0xffffffffu:
            return 1;

        default:
            return 2;
    }
}

int test4(unsigned long x) {
    switch (x) {
        case 0xffffffffffffff00ull ... 0xffffffffffffffffull:
            return 1;

        default:
            return 2;
    }
}

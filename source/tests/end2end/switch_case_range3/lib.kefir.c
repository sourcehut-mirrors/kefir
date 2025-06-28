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

int test1(unsigned _BitInt(6) x) {
    switch (x) {
        case 0xf0uwb ... 0xffuwb:
            return 1;

        default:
            return 2;
    }
}

int test2(unsigned _BitInt(14) x) {
    switch (x) {
        case 0xff00uwb ... 0xffffuwb:
            return 1;

        default:
            return 2;
    }
}

int test3(unsigned _BitInt(29) x) {
    switch (x) {
        case 0xffffff00uwb ... 0xffffffffuwb:
            return 1;

        default:
            return 2;
    }
}

int test4(unsigned _BitInt(60) x) {
    switch (x) {
        case 0xffffffffffffff00uwb ... 0xffffffffffffffffuwb:
            return 1;

        default:
            return 2;
    }
}

int test5(unsigned _BitInt(120) x) {
    switch (x) {
        case 0xffffffffffffff00uwb ... 0xffffffffffffffffuwb:
            return 1;

        default:
            return 2;
    }
}

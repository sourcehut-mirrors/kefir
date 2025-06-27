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

int test1(_BitInt(6) x) {
    switch (x) {
        case 0wb ... 9wb:
            return 1;

        case 10wb ... 19wb:
            return 2;

        case 20wb ... 29wb:
            return 3;

        default:
            return 4;
    }
}

int test2(_BitInt(14) x) {
    switch (x) {
        case 0wb ... 99wb:
            return 1;

        case 100wb ... 1999wb:
            return 2;

        case 2000wb ... 2999wb:
            return 3;

        default:
            return 4;
    }
}

int test3(_BitInt(29) x) {
    switch (x) {
        case 0wb ... 999wb:
            return 1;

        case 1000wb ... 3999wb:
            return 2;

        case 4000wb ... 10000wb:
            return 3;

        default:
            return 4;
    }
}

int test4(_BitInt(60) x) {
    switch (x) {
        case 0wb ... 999wb:
            return 1;

        case 1000wb ... 3999wb:
            return 2;

        case 4000wb ... 10000wb:
            return 3;

        default:
            return 4;
    }
}

int test5(_BitInt(120) x) {
    switch (x) {
        case 0wb ... 0x0000222211114444fffewb:
            return 1;

        case 0x0000222211114444ffffwb ... 0xaaaa222211114444fffewb:
            return 2;

        case 0xaaaa222211114444ffffwb ... 0xffff222211114444ffffwb:
            return 3;

        default:
            return 4;
    }
}

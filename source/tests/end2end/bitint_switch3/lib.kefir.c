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
        case 9wb ... 0wb:
            return 1;

        case 19wb ... 10wb:
            return 2;

        case 29wb ... 20wb:
            return 3;

        default:
            return 4;
    }
}

int test2(_BitInt(14) x) {
    switch (x) {
        case 99wb ... 0wb:
            return 1;

        case 1999wb ... 100wb:
            return 2;

        case 2999wb ... 2000wb:
            return 3;

        default:
            return 4;
    }
}

int test3(_BitInt(29) x) {
    switch (x) {
        case 999wb ... 0wb:
            return 1;

        case 3999wb ... 1000wb:
            return 2;

        case 10000wb ... 4000wb:
            return 3;

        default:
            return 4;
    }
}

int test4(_BitInt(60) x) {
    switch (x) {
        case 999wb ... 0wb:
            return 1;

        case 3999wb ... 1000wb:
            return 2;

        case 10000wb ... 4000wb:
            return 3;

        default:
            return 4;
    }
}

int test5(_BitInt(120) x) {
    switch (x) {
        case 0x0000222211114444fffewb ... 0wb:
            return 1;

        case 0xaaaa222211114444fffewb ... 0x0000222211114444ffffwb:
            return 2;

        case 0xffff222211114444ffffwb ... 0xaaaa222211114444ffffwb:
            return 3;

        default:
            return 4;
    }
}

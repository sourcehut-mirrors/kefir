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

unsigned long get(int x) {
    switch (x) {
        case 0:
            return 0wb;

        case 1:
            return 0uwb;

        case 2:
            return 1WB;

        case 3:
            return 1wbU;

        case 4:
            return -1wb;

        case 5:
            return 7uWB;

        case 6:
            return 15wb;

        case 7:
            return -16wb;

        case 8:
            return 16Uwb;

        case 9:
            return 0x7fffwb;

        case 10:
            return 0x8000wb;

        case 11:
            return 0x8000uwb;

        case 12:
            return 0xffffwb;

        case 13:
            return 0xffffuwb;

        case 14:
            return 0x7fffffffffffffffwb;

        case 15:
            return 0x8000000000000000wb;

        case 16:
            return 0xffffffffffffffffuwb;
    }

    return -0xcafebabe;
}

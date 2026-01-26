/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2026  Jevgenijs Protopopovs

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

_Complex float get(int x, int a, _Complex float b, _Complex float c) {
    if (x)
        b = a == 1 ? b : c;
    return b;
}

_Complex float get2(int x, int a, _Complex float b, _Complex float c) {
    if (x)
        b = a != 2 ? b : c;
    return b;
}

_Complex float get3(int x, int a, _Complex float b, _Complex float c) {
    if (x)
        b = a > 3 ? b : c;
    return b;
}

_Complex float get4(int x, int a, _Complex float b, _Complex float c) {
    if (x)
        b = a >= 4 ? b : c;
    return b;
}

_Complex float get5(int x, int a, _Complex float b, _Complex float c) {
    if (x)
        b = a < -3 ? b : c;
    return b;
}

_Complex float get6(int x, int a, _Complex float b, _Complex float c) {
    if (x)
        b = a <= -4 ? b : c;
    return b;
}

_Complex float get7(int x, unsigned int a, _Complex float b, _Complex float c) {
    if (x)
        b = a > 30 ? b : c;
    return b;
}

_Complex float get8(int x, unsigned int a, _Complex float b, _Complex float c) {
    if (x)
        b = a >= 40 ? b : c;
    return b;
}

_Complex float get9(int x, unsigned int a, _Complex float b, _Complex float c) {
    if (x)
        b = a < 100 ? b : c;
    return b;
}

_Complex float get10(int x, unsigned int a, _Complex float b, _Complex float c) {
    if (x)
        b = a <= 200 ? b : c;
    return b;
}

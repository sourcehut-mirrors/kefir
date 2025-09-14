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

long double get(long x, long a, long double b, long double c) {
        if (x)  b = a == 1 ? b : c;
        return b;
}

long double get2(long x, long a, long double b, long double c) {
        if (x)  b = a != 2 ? b : c;
        return b;
}

long double get3(long x, long a, long double b, long double c) {
        if (x)  b = a > 3 ? b : c;
        return b;
}

long double get4(long x, long a, long double b, long double c) {
        if (x)  b = a >= 4 ? b : c;
        return b;
}

long double get5(long x, long a, long double b, long double c) {
        if (x)  b = a < -3 ? b : c;
        return b;
}

long double get6(long x, long a, long double b, long double c) {
        if (x)  b = a <= -4 ? b : c;
        return b;
}

long double get7(long x, unsigned long a, long double b, long double c) {
        if (x)  b = a > 30 ? b : c;
        return b;
}

long double get8(long x, unsigned long a, long double b, long double c) {
        if (x)  b = a >= 40 ? b : c;
        return b;
}

long double get9(long x, unsigned long a, long double b, long double c) {
        if (x)  b = a < 100 ? b : c;
        return b;
}

long double get10(long x, unsigned long a, long double b, long double c) {
        if (x)  b = a <= 200 ? b : c;
        return b;
}

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

int test1(void) {
    int a;
    return a;
}

long test2(long a) {
    long x;
    long y = a > 0 ? (x = a) : -1;
    if (y != -1) {
        return x;
    } else {
        return a;
    }
}

float test3(float x) {
    float y;
    if (x > 0.0f) {
        y = x;
    }
    return y;
}

double test4(double x) {
    double y;
    if (x > 0.0) {
        y = x;
    }
    return y;
}

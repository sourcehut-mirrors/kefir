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

int x[3];
_Thread_local int y[3];

int test1(int a) {
    x[0] = a;
    x[1] = a + 1;
    x[2] = a * 2;
    return x[0] + x[1] + x[2];
}

int test2(int a) {
    y[0] = a;
    y[1] = a + 1;
    y[2] = a * 2;
    return y[0] + y[1] + y[2];
}

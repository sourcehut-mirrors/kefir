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

int a[] = (const int[]) {100, 20, 3, -4};
int b = sizeof(a) / sizeof(a[0]);

const long c[10] = (const long[]) {-1, -2, -3};
const long d = sizeof(c) / sizeof(c[0]);

struct S1 e[] = (volatile struct S1[3]) {{-3.14f, 4}};
long f = sizeof(e) / sizeof(e[0]);

/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2025  Jevgenijs Protopopovs

    This file is part of Kefir project.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "./definitions.h"

void *const ptr1 = (void *) (((char *) (int[]) {-100, -200, 1, 2, 3, 4, 5}) + (sizeof(int) * 2));

void **const ptr2 = (void **) {&ptr1};

long scalar1 = (int) {-1234};
double scalar2 = (double) (float) {3.14159f};
_Complex double complex1 = (_Complex double) {2.1828};

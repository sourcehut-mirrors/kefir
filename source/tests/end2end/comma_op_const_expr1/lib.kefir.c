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

enum { Y = (1, 2, (2, 3, (3, (4, 5)))) };

int x = (0, (1, 2, 3, 4), (5, 6, 7, (8, 9)), (10, (11, (12, 13))));

int y = Y;

double z = ((0.0, 1.0, (2.0, (3.0, (4.0, 5.0))), 6.0));

_Complex float w = (1.0 + 1.0i, 0.0 - 1.0i, (0.0 + 0.0i, 1.0 - 1.0i));

void *a = (0, &w, (&y, &x));

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

struct S2 s2 = {1, {2, 3, 4, 5, 6, 7, 8, 9}, 10, 11, 12, 13, 14, 15, 16, {17, 18, 19, 20, 21, 22, 23, 24}};

_Static_assert(sizeof(struct S2) == 145);
_Static_assert(_Alignof(struct S2) == 1);

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

#define OFFSETOF(x, y) __builtin_offsetof(x, y)

const unsigned long descriptor[] = {sizeof(struct S1),
                                    _Alignof(struct S1),
                                    OFFSETOF(struct S1, a),
                                    OFFSETOF(struct S1, b),
                                    OFFSETOF(struct S1, c),
                                    OFFSETOF(struct S1, g),
                                    OFFSETOF(struct S1, h),
                                    OFFSETOF(struct S1, i),
                                    OFFSETOF(struct S1, j),
                                    OFFSETOF(struct S1, k),
                                    OFFSETOF(struct S1, l),
                                    OFFSETOF(struct S1, m),
                                    OFFSETOF(struct S1, m) + OFFSETOF(__typeof((struct S1){0}.m), a),
                                    OFFSETOF(struct S1, m) + OFFSETOF(__typeof((struct S1){0}.m), b),
                                    OFFSETOF(struct S1, m) + OFFSETOF(__typeof((struct S1){0}.m), c),
                                    OFFSETOF(struct S1, n),
                                    OFFSETOF(struct S1, o),
                                    OFFSETOF(struct S1, p),
                                    OFFSETOF(struct S1, q)};

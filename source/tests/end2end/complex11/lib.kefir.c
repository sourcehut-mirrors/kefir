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

_Complex float test32(_Complex float x) {
    _Complex float y;
    asm volatile("movaps %1, %0\n"
                 "shufps $1, %0, %0"
                 : "=x"(y)
                 : "x"(x));
    return y;
}

_Complex double test64(_Complex double x) {
    _Complex double y;
    asm volatile("movapd %1, %0\n"
                 "shufpd $1, %0, %0"
                 : "=x"(y)
                 : "x"(x));
    return y;
}

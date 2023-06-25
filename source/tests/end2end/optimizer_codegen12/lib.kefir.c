/*
    SPDX-License-Identifier: GPL-3.0

    Copyright (C) 2020-2023  Jevgenijs Protopopovs

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

long geta(const struct Struct1 *s) {
    return s->a;
}

long getb(const struct Struct1 *s) {
    return s->b;
}

unsigned int getx(const struct Struct1 *s) {
    return s->x;
}

unsigned int gety(const struct Struct1 *s) {
    return s->y;
}

void seta(struct Struct1 *s, long x) {
    s->a = x;
}

void setb(struct Struct1 *s, long x) {
    s->b = x;
}

void setx(struct Struct1 *s, unsigned int x) {
    s->x = x;
}

void sety(struct Struct1 *s, unsigned int x) {
    s->y = x;
}

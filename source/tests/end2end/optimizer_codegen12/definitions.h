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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

struct Struct1 {
    long : 6, a : 19, b : 5;
    unsigned int x : 13, : 6, y : 10;
};

long geta(const struct Struct1 *);
long getb(const struct Struct1 *);
unsigned int getx(const struct Struct1 *);
unsigned int gety(const struct Struct1 *);

void seta(struct Struct1 *, long);
void setb(struct Struct1 *, long);
void setx(struct Struct1 *, unsigned int);
void sety(struct Struct1 *, unsigned int);

#endif

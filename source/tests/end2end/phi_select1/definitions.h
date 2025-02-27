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

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

struct S1 {
    char buf[128];
};

int select1(int, int, int);
long select2(int, long, long);
float select3(int, float, float);
double select4(int, double, double);
long double select5(int, long double, long double);
_Complex float select6(int, _Complex float, _Complex float);
_Complex double select7(int, _Complex double, _Complex double);
_Complex long double select8(int, _Complex long double, _Complex long double);
struct S1 select9(int, struct S1, struct S1);

#endif

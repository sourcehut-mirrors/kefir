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

int select1(int x, int y, int z) {
    return x ? y : z;
}

long select2(int x, long y, long z) {
    return x ? y : z;
}

float select3(int x, float y, float z) {
    return x ? y : z;
}

double select4(int x, double y, double z) {
    return x ? y : z;
}

long double select5(int x, long double y, long double z) {
    return x ? y : z;
}

_Complex float select6(int x, _Complex float y, _Complex float z) {
    return x ? y : z;
}

_Complex double select7(int x, _Complex double y, _Complex double z) {
    return x ? y : z;
}

_Complex long double select8(int x, _Complex long double y, _Complex long double z) {
    return x ? y : z;
}

struct S1 select9(int x, struct S1 y, struct S1 z) {
    return x ? y : z;
}

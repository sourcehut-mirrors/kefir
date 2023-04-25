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

extern int sum(int, int) asm("add2");
extern float fsum(float, float) asm("fadd2");
extern float fsum3(float, float, float) asm("fadd3");

int (*fnsum)(int, int) = sum;
float (*fnfsum)(float, float) = fsum;

int sum(int a, int b) {
    return a + b;
}

float fsum3(float x, float y, float z) {
    return fsum(x, fsum(y, z));
}

int (*getsum())(int, int) {
    return sum;
}

float (*getfsum())(float, float) {
    return fsum;
}

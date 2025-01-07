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

#ifdef __x86_64__
#define SUB(_a, _b) asm("fsubp" : "+t"(_a) : "u"(b))

extern float subf(float a, float b) {
    SUB(a, b);
    return a;
}

extern double sub(double a, double b) {
    SUB(a, b);
    return a;
}

extern long double subl(long double a, long double b) {
    SUB(a, b);
    return a;
}

#define DIV(_a, _b) asm("fdivp" : "+t"(_a) : "f"(b))

extern float mydivf(float a, float b) {
    DIV(a, b);
    return a;
}

extern double mydiv(double a, double b) {
    DIV(a, b);
    return a;
}

extern long double mydivl(long double a, long double b) {
    DIV(a, b);
    return a;
}

#define DIVR(_a, _b) asm("fdivrp" : "+f"(_a) : "u"(b))

extern float mydivrf(float a, float b) {
    DIVR(a, b);
    return a;
}

extern double mydivr(double a, double b) {
    DIVR(a, b);
    return a;
}

extern long double mydivrl(long double a, long double b) {
    DIVR(a, b);
    return a;
}
#endif

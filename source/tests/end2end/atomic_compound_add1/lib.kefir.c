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

#if !defined(__DragonFly__) || defined(KEFIR_END2END_ASMGEN)
void add8(_Atomic char *x, char y) {
    *x += y;
}

void add16(_Atomic short *x, short y) {
    *x += y;
}

void add32(_Atomic int *x, int y) {
    *x += y;
}

void add64(_Atomic long *x, long y) {
    *x += y;
}

void addf32(_Atomic float *x, float y) {
    *x += y;
}

void addf64(_Atomic double *x, double y) {
    *x += y;
}

char add8r(_Atomic char *x, char y) {
    return *x += y;
}

short add16r(_Atomic short *x, short y) {
    return *x += y;
}

int add32r(_Atomic int *x, int y) {
    return *x += y;
}

long add64r(_Atomic long *x, long y) {
    return *x += y;
}

float addf32r(_Atomic float *x, float y) {
    return *x += y;
}

double addf64r(_Atomic double *x, double y) {
    return *x += y;
}
#endif

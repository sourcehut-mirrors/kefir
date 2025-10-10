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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

extern float a;
extern double b;
extern double c;
extern long double d;
extern long double e;

extern float mygeta(void);
extern double mygetb(void);
extern double mygetc(void);
extern long double mygetd(void);
extern long double mygete(void);

int main(void) {
    assert(fabs(a - 3.14159) < 1e-6);
    assert(fabs(b - 2.71828) < 1e-8);
    assert(fabs(c - 3.14159) < 1e-8);
    assert(fabsl(d - 2.71828L) < 1e-9);
    assert(fabsl(e - 3.14159L) < 1e-9);

    assert(fabs(mygeta() - 3.14159) < 1e-6);
    assert(fabs(mygetb() - 2.71828) < 1e-8);
    assert(fabs(mygetc() - 3.14159) < 1e-8);
    assert(fabsl(mygetd() - 2.71828L) < 1e-9);
    assert(fabsl(mygete() - 3.14159L) < 1e-9);
    return EXIT_SUCCESS;
}

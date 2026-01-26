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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <complex.h>

extern _Complex float a;
extern _Complex double b;
extern _Complex double c;
extern _Complex long double d;
extern _Complex long double e;

extern _Complex float mygeta(void);
extern _Complex double mygetb(void);
extern _Complex double mygetc(void);
extern _Complex long double mygetd(void);
extern _Complex long double mygete(void);

int main(void) {
    assert(fabs(cimag(a) - 3.14159) < 1e-6);
    assert(fabs(cimag(b) - 2.71828) < 1e-8);
    assert(fabs(cimag(c) - 3.14159) < 1e-8);
    assert(fabsl(cimagl(d) - 2.71828L) < 1e-9);
    assert(fabsl(cimagl(e) - 3.14159L) < 1e-9);

    assert(fabs(cimag(mygeta()) - 3.14159) < 1e-6);
    assert(fabs(cimag(mygetb()) - 2.71828) < 1e-8);
    assert(fabs(cimag(mygetc()) - 3.14159) < 1e-8);
    assert(fabsl(cimagl(mygetd()) - 2.71828L) < 1e-9);
    assert(fabsl(cimagl(mygete()) - 3.14159L) < 1e-9);
    return EXIT_SUCCESS;
}

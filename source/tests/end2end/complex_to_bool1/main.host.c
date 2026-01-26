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
#include "./definitions.h"

int main(void) {
    int i;

    assert(b0[0]);
    assert(!b0[1]);

    i = 0;
    assert(!b32[i++]);
    assert(!b32[i++]);
    assert(!b32[i++]);
    assert(b32[i++]);
    assert(b32[i++]);
    assert(b32[i++]);
    assert(b32[i++]);

    i = 0;
    assert(!b32_2[i++]);
    assert(!b32_2[i++]);
    assert(!b32_2[i++]);
    assert(b32_2[i++]);
    assert(b32_2[i++]);
    assert(b32_2[i++]);
    assert(b32_2[i++]);

    i = 0;
    assert(!b64[i++]);
    assert(!b64[i++]);
    assert(!b64[i++]);
    assert(b64[i++]);
    assert(b64[i++]);
    assert(b64[i++]);
    assert(b64[i++]);

    i = 0;
    assert(!b64_2[i++]);
    assert(!b64_2[i++]);
    assert(!b64_2[i++]);
    assert(b64_2[i++]);
    assert(b64_2[i++]);
    assert(b64_2[i++]);
    assert(b64_2[i++]);

    i = 0;
    assert(!b64_3[i++]);
    assert(!b64_3[i++]);
    assert(!b64_3[i++]);
    assert(b64_3[i++]);
    assert(b64_3[i++]);
    assert(b64_3[i++]);
    assert(b64_3[i++]);

    i = 0;
    assert(!b80[i++]);
    assert(!b80[i++]);
    assert(!b80[i++]);
    assert(b80[i++]);
    assert(b80[i++]);
    assert(b80[i++]);
    assert(b80[i++]);

    i = 0;
    assert(!b80_2[i++]);
    assert(!b80_2[i++]);
    assert(!b80_2[i++]);
    assert(b80_2[i++]);
    assert(b80_2[i++]);
    assert(b80_2[i++]);
    assert(b80_2[i++]);

    i = 0;
    assert(!b80_3[i++]);
    assert(!b80_3[i++]);
    assert(!b80_3[i++]);
    assert(b80_3[i++]);
    assert(b80_3[i++]);
    assert(b80_3[i++]);
    assert(b80_3[i++]);

    assert(!c32_to_bool((_Complex float) (0.0f)));
    assert(!c32_to_bool((_Complex float) (0.0f + 0.0f * I)));
    assert(!c32_to_bool((_Complex float) (0.0f * I)));
    assert(c32_to_bool((_Complex float) (1.0f)));
    assert(c32_to_bool((_Complex float) (1.0f + 0.0f * I)));
    assert(c32_to_bool((_Complex float) (1.0f * I)));
    assert(c32_to_bool((_Complex float) (1.0f + 1.0f * I)));

    assert(!c32_to_bool2((_Complex float) (0.0f)));
    assert(!c32_to_bool2((_Complex float) (0.0f + 0.0f * I)));
    assert(!c32_to_bool2((_Complex float) (0.0f * I)));
    assert(c32_to_bool2((_Complex float) (1.0f)));
    assert(c32_to_bool2((_Complex float) (1.0f + 0.0f * I)));
    assert(c32_to_bool2((_Complex float) (1.0f * I)));
    assert(c32_to_bool2((_Complex float) (1.0f + 1.0f * I)));

    assert(!c64_to_bool((_Complex double) (0.0f)));
    assert(!c64_to_bool((_Complex double) (0.0f + 0.0f * I)));
    assert(!c64_to_bool((_Complex double) (0.0f * I)));
    assert(c64_to_bool((_Complex double) (1.0f)));
    assert(c64_to_bool((_Complex double) (1.0f + 0.0f * I)));
    assert(c64_to_bool((_Complex double) (1.0f * I)));
    assert(c64_to_bool((_Complex double) (1.0f + 1.0f * I)));

    assert(!c64_to_bool2((_Complex double) (0.0f)));
    assert(!c64_to_bool2((_Complex double) (0.0f + 0.0f * I)));
    assert(!c64_to_bool2((_Complex double) (0.0f * I)));
    assert(c64_to_bool2((_Complex double) (1.0f)));
    assert(c64_to_bool2((_Complex double) (1.0f + 0.0f * I)));
    assert(c64_to_bool2((_Complex double) (1.0f * I)));
    assert(c64_to_bool2((_Complex double) (1.0f + 1.0f * I)));

    assert(!c64_to_bool3((_Complex double) (0.0f)));
    assert(!c64_to_bool3((_Complex double) (0.0f + 0.0f * I)));
    assert(!c64_to_bool3((_Complex double) (0.0f * I)));
    assert(c64_to_bool3((_Complex double) (1.0f)));
    assert(c64_to_bool3((_Complex double) (1.0f + 0.0f * I)));
    assert(c64_to_bool3((_Complex double) (1.0f * I)));
    assert(c64_to_bool3((_Complex double) (1.0f + 1.0f * I)));

    assert(!c80_to_bool((_Complex long double) (0.0f)));
    assert(!c80_to_bool((_Complex long double) (0.0f + 0.0f * I)));
    assert(!c80_to_bool((_Complex long double) (0.0f * I)));
    assert(c80_to_bool((_Complex long double) (1.0f)));
    assert(c80_to_bool((_Complex long double) (1.0f + 0.0f * I)));
    assert(c80_to_bool((_Complex long double) (1.0f * I)));
    assert(c80_to_bool((_Complex long double) (1.0f + 1.0f * I)));

    assert(!c80_to_bool2((_Complex long double) (0.0f)));
    assert(!c80_to_bool2((_Complex long double) (0.0f + 0.0f * I)));
    assert(!c80_to_bool2((_Complex long double) (0.0f * I)));
    assert(c80_to_bool2((_Complex long double) (1.0f)));
    assert(c80_to_bool2((_Complex long double) (1.0f + 0.0f * I)));
    assert(c80_to_bool2((_Complex long double) (1.0f * I)));
    assert(c80_to_bool2((_Complex long double) (1.0f + 1.0f * I)));

    assert(!c80_to_bool3((_Complex long double) (0.0f)));
    assert(!c80_to_bool3((_Complex long double) (0.0f + 0.0f * I)));
    assert(!c80_to_bool3((_Complex long double) (0.0f * I)));
    assert(c80_to_bool3((_Complex long double) (1.0f)));
    assert(c80_to_bool3((_Complex long double) (1.0f + 0.0f * I)));
    assert(c80_to_bool3((_Complex long double) (1.0f * I)));
    assert(c80_to_bool3((_Complex long double) (1.0f + 1.0f * I)));
    return EXIT_SUCCESS;
}
